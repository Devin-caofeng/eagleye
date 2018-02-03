#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>

#include "../include/scan_dir.h"
#include "../include/monitor.h"
#include "../include/queue.h"
#include "../include/list.h"
#include "../include/send.h"
#include "../include/user.h"
#include "../include/wrap.h"
#include "../include/ctrl.h"
#include "../include/log.h"


#define MAX_LINE      80
#define SERV_PORT     9000
#define BACK_LOG      66
#define INET_ADDR_LEN 120
#define SERV_IP       "192.168.223.133"

typedef struct sockaddr SA;

int main(int argv, char *args[]) {

    if (argv < 3) {
        printf("plase input name: , passwd: \n");
        return -1;
    }

    int serv_fd = Socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serv_addr;
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    inet_pton(AF_INET, SERV_IP, &serv_addr.sin_addr);
    serv_addr.sin_port = htons(SERV_PORT);

    Connect(serv_fd, (SA *)&serv_addr, sizeof(serv_addr));

    ReqHead req_head;
    req_head.type = REQ_LOGIN;
    req_head.len = sizeof(ReqLogin);
    if (Send(serv_fd, (char *)&req_head, sizeof(req_head), 0) < 0) {
        perror("send req_head error");
        return -1;
    }

    ReqLogin req_login;
    strcpy(req_login.user_name, args[1]);
    strcpy(req_login.user_passwd, args[2]);
    strcpy(req_login.user_path, args[3]);
    if (Send(serv_fd, (char *)&req_login, sizeof(req_login), 0) < 0) {
        perror("send req_login error");
        return -1;
    }

    ReqHead rsp_comm;
    if (recv(serv_fd, (char *)&rsp_comm, sizeof(rsp_comm), 0) < 0) {
        perror("recv rsp_comm error");
        return -1;
    }

    char buf[MAX_LINE];
    while (fgets(buf, MAX_LINE, stdin) != NULL) {
        Write(serv_fd, buf, strlen(buf));
        // int n = Read(serv_fd, buf, MAX_LINE);
        int n = recv(serv_fd, buf, MAX_LINE, 0);

        if (n == 0) {
            printf("the other side has been closed\n");
            break;
        }
        else {
            Write(STDOUT_FILENO, buf, n);
        }
    }

    Close(serv_fd);

    return 0;
}
