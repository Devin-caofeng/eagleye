#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>

#include "../include/log.h"
#include "../include/queue.h"
#include "../include/list.h"
#include "../include/send.h"
#include "../include/user.h"
#include "../include/wrap.h"
#include "../include/ctrl.h"
#include "../include/monitor.h"
#include "../include/scan_dir.h"


#define MAX_LINE      80
#define SERV_PORT     9000
#define SERV_IP       "192.168.42.128"

typedef struct sockaddr SA;

int Login(int num, char *account[], int serv_fd);
int UploadFile(int serv_fd);

int main(int argv, char *args[]) {

    if (argv < 3) {
        printf("plase input name:, passwd: \n");
        return -1;
    }

    int serv_fd = Socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serv_addr;
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    inet_pton(AF_INET, SERV_IP, &serv_addr.sin_addr);
    serv_addr.sin_port = htons(SERV_PORT);

    Connect(serv_fd, (SA *)&serv_addr, sizeof(serv_addr));

    Login(argv, args, serv_fd);

    // 上传文件数据
    while (1) {
        UploadFile(serv_fd);
        sleep(2);
    }

    Close(serv_fd);

    return 0;
}

int Login(int argv, char *account[], int serv_fd) {
    // 发送登录请求
    ReqHead req_head;
    req_head.type = REQ_LOGIN;
    req_head.len = sizeof(ReqLogin);
    if (Send(serv_fd, (char *)&req_head, sizeof(req_head), 0) < 0) {
        perror("send req_head error");
        return -1;
    }

    // 发送账号信息
    ReqLogin req_login;
    strcpy(req_login.user_name, account[1]);
    strcpy(req_login.user_passwd, account[2]);
    strcpy(req_login.user_group, account[3]);
    if (argv > 4) {
        // 检测是否注册
        if (strcmp(account[4], "register") == 0) req_login.regis= 1;
        else req_login.regis = 0;
    }

    if (Send(serv_fd, (char *)&req_login, sizeof(req_login), 0) < 0) {
        perror("send req_login error");
        return -1;
    }

    // 接收登录结果
    ReqHead rsp_comm;
    if (recv(serv_fd, (char *)&rsp_comm, sizeof(rsp_comm), 0) < 0) {
        perror("recv rsp_comm error");
        return -1;
    }
    if (rsp_comm.type != RSP_LOGIN || rsp_comm.len != 0) {
        printf("login error, plase try again\n");
        return -1;
    }

    return 0;
}

static int SendUploadRequestHead(int serv_fd, const char *file_name) {
    ReqHead req_head;
    req_head.type = SEND_FILE;
    req_head.len = sizeof(ReqFile);
    if (Send(serv_fd, &req_head, sizeof(req_head), 0) == -1) {
        printf("%s:%d: send req_head error\n", __FILE__, __LINE__);
        return -1;
    }

    struct stat st;
    if (stat(file_name, &st) == -1) {
        printf("%s:%d: get file status error\n", __FILE__, __LINE__);
        return -1;
    }

    ReqFile req_file;
    req_file.file_len = st.st_size;
    strncpy(req_file.path, file_name, strlen(file_name));
    if (Send(serv_fd, &req_file, req_head.len, 0) == -1) {
        printf("%s:%d: send req_file error\n", __FILE__, __LINE__);
        return -1;
    }

    return 0;
}

static int SendFile(int fd, int serv_fd) {
    char buf[BUF_LEN];
    memset(buf, '\0', BUF_LEN);

    int ret = 0;
    do {
        ret = Read(fd, buf, BUF_LEN);
        if (ret < 0) {
            printf("%s:%d: read file error\n", __FILE__, __LINE__);
            return -1;
        }
        else if (ret == 0) {
            return 0;
        }
        printf("client: read %d btye\n", ret);

        ret = Send(serv_fd, buf, ret, 0);
        printf("client: send to serv %d btye\n", ret);
        if (ret < 0) {
            printf("%s:%d: send file error\n", __FILE__, __LINE__);
            return -1;
        }
    } while (ret > 0);

    return 0;
}

int UploadFile(int serv_fd) {

    // 递归扫描文件，然后进行发送

    const char *file_name = "/home/cf/time.md";

    SendUploadRequestHead(serv_fd, file_name);

    int send_file_fd = open(file_name, O_RDONLY);

    SendFile(send_file_fd, serv_fd);

    return 0;
}
