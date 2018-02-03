#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <libconfig.h>

#include "../include/scan_dir.h"
#include "../include/monitor.h"
#include "../include/queue.h"
#include "../include/list.h"
#include "../include/send.h"
#include "../include/user.h"
#include "../include/wrap.h"
#include "../include/ctrl.h"
#include "../include/log.h"

#define MAX_LISTEN 1024
#define ADDR_LEN   128
#define FILE_LEN   256
#define BACK_LOG   1024

typedef struct sockaddr SA;

int daemon_flag = 1;
char conf[] = "./etc/init.conf";
static char serv_ip[ADDR_LEN];
static int  serv_port = -1;
static char log_file[FILE_LEN];

List *login_send = NULL;
List *no_login= NULL;


int SetNonblock(int fd);
int InitOption(int argc, char *argv[]);
int InitSignal();
int ParseConf(const char *path);
int CheckLoginRequest(int fd, UserInfo *user_info);
int ListenLogin();


int main(int argc, char *argv[]) {

    InitOption(argc, argv);
    if (daemon_flag) daemon(1, 0);

    InitSignal();

    ParseConf(conf);

    if (daemon_flag) {
        if (OpenLog(log_file, "f_cat") < 0) return -1;
    }
    else {
        if (OpenLog(log_file, "o_cat") < 0) return -1;
    }

    StartScanDirThread(NULL);

    StartSendThread();

    ListenLogin();

    return 0;
}


int SetNonblock(int fd) {
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

int InitOption(int argc, char *argv[]) {
    const char *flag= "d";
    char ch;
    while ((ch = getopt(argc, argv, flag)) != -1) {
        switch (ch) {
            case 'd':
                daemon_flag = 0;
                break;
            default:
                printf("usage: \n\t-d\t\t into debug mode!\n");
        }
    }

    return 0;
}

int InitSignal() {
    signal(SIGPIPE, SIG_IGN);
    return 0;
}

int ParseConf(const char *path) {
    if (path == NULL) {
        printf("config path is null\n");
        return -1;
    }

    config_t conf;
    config_init(&conf);
    if (!config_read_file(&conf, path)) {
        printf("read config file error!\n");
        return -1;
    }

    const char *lookup_ret = NULL;
    if (!config_lookup_string(&conf, "conf.log_file", &lookup_ret)) {
        printf("get log file path error\n");
        return -1;
    }
    memcpy(log_file, lookup_ret, strlen(lookup_ret));

    if (!config_lookup_string(&conf, "conf.serv_ip", &lookup_ret)) {
        printf("get server addr error\n");
        return -1;
    }
    memcpy(serv_ip, lookup_ret, strlen(lookup_ret));

    if (!config_lookup_int(&conf, "conf.serv_port", &serv_port)) {
        printf("get server prot error\n");
        return -1;
    }

    printf("log:[%s], addr[%s], port[%d]\n", log_file, serv_ip, serv_port);

    config_destroy(&conf);

    return 0;
}

/*
 * 功能：检测用户登录请求
 *     1. 接收用户登录请求头
 *     2. 判断此请求是否为登录请求，如不是直接返回-1
 *     3. 接收用户登录信息
 *     4。查询数据库，验证登录信息
 *     5。
 *     6。向用户发送登录成功信息
 *
 * 返回值:
 *      0 表示成功
 *     -1 表示失败
 */
int CheckLoginRequest(int sockfd, UserInfo *user_info) {
    ReqHead req_head;
    if (Recv(sockfd, (char *)&req_head, sizeof(req_head), 0) < 0) {
        ERR("recv request head error!");
        return -1;
    }
    if (req_head.type != REQ_LOGIN) return -1;

    ReqLogin req_login;
    if (Recv(sockfd, (char *)&req_login, req_head.len, 0) < 0) {
        ERR("recv request head error!");
        return -1;
    }
    DBG("login client:[%s-%s]", req_login.user_name, req_login.user_passwd);
    strcpy(user_info->dir, req_login.user_path);
    // 从数据库中查询

    ReqHead rsp_comm;
    rsp_comm.type = RSP_LOGIN;
    rsp_comm.len = 0;
    if (Send(sockfd, (char *)&rsp_comm, sizeof(rsp_comm), 0) < 0) {
        ERR("send request head error!");
        return -1;
    }

    return 0;
}

/*
 * 功能：监听用户登录过程
 *
 */
int ListenLogin() {
    int listenfd = Socket(AF_INET, SOCK_STREAM, 0);
    socklen_t option = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    inet_pton(AF_INET, serv_ip, &serv_addr.sin_addr);
    serv_addr.sin_port = htons(serv_port);

    Bind(listenfd, (SA *)&serv_addr, sizeof(serv_addr));

    Listen(listenfd, BACK_LOG);

    SetNonblock(listenfd);

    int epollfd = epoll_create(MAX_LISTEN);
    if (epollfd < 0) {
        ERR("epoll create error!");
        return -1;
    }

    struct epoll_event events[MAX_LISTEN], event;
    event.data.fd = listenfd;
    event.events = EPOLLIN;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &event);

    while (1) {
        List *cur_list_node = NULL;
        UserInfo *cur_user_info = NULL;

        // 遍历还未成功登录的用户链表
        for (cur_list_node = no_login;
                 cur_list_node != NULL;
                 cur_list_node = cur_list_node->next) {
            cur_user_info = (UserInfo *)cur_list_node->data;
            time_t t = time(NULL);

            // 建立连接后，三秒内还未成功登录则从监听描述符集合中删除，并关闭
            // 此用户打开的文件描述符，最后从未成功登录链表中移除
            if ((t - cur_user_info->conn_time) > 66) {
                epoll_ctl(epollfd, EPOLL_CTL_DEL, cur_user_info->sock_fd, NULL);
                close(cur_user_info->file_fd);
                free(cur_user_info->ptr);
                no_login = DelFromList(no_login, cur_user_info);
                ERR("client login time out");
            }
        }

        int nfds = epoll_wait(epollfd, events, MAX_LISTEN, 3000);
        if (nfds < 0) {
            usleep(1000);
            continue;
        }
        else if (nfds == 0) {
            continue;
        }

        int i;
        // 遍历有事件发生的描述符集合
        for (i = 0; i < nfds; ++i) {
            if (events[i].data.fd == listenfd) {
                struct sockaddr_in cli_addr;
                socklen_t cli_addr_len = sizeof(cli_addr);
                int cli_fd = Accept(listenfd, (SA *)&cli_addr, &cli_addr_len);

                SetNonblock(cli_fd);
                UserInfo *new_user = AddUser();
                new_user->conn_time = time(NULL);
                new_user->sock_fd = cli_fd;
                no_login = AddToListTail(no_login, new_user);

                event.data.ptr = new_user;
                event.events = EPOLLIN;
                epoll_ctl(epollfd, EPOLL_CTL_ADD, cli_fd, &event);
                DBG("new conn:[%s]-[%d],conn_time:[%ld]",
                     inet_ntoa(cli_addr.sin_addr),
                     ntohs(cli_addr.sin_port), new_user->conn_time);
            }
            else if (events[i].events & EPOLLIN) {
                UserInfo *new_user = (UserInfo *)events[i].data.ptr;
                if (CheckLoginRequest(new_user->sock_fd, new_user) < 0) {
                    epoll_ctl(epollfd, EPOLL_CTL_DEL, new_user->sock_fd, NULL);
                    close(new_user->file_fd);
                    no_login = DelFromList(no_login, new_user);
                    free(new_user);
                }
                no_login = DelFromList(no_login, new_user);
                login_send = AddToListTail(login_send, new_user);
                epoll_ctl(epollfd, EPOLL_CTL_DEL, new_user->sock_fd, NULL);
            }
            else {
                UserInfo *new_user = (UserInfo *)events[i].data.ptr;
                epoll_ctl(epollfd, EPOLL_CTL_DEL, new_user->sock_fd, NULL);
                close(new_user->file_fd);
            }
        }
    }

    return 0;
}
