#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "../include/log.h"
#include "../include/list.h"
#include "../include/recv.h"
#include "../include/user.h"
#include "../include/wrap.h"
#include "../include/queue.h"
#include "../include/monitor.h"

#define Print printf("%s->%d\n", __FILE__, __LINE__);

static int RecvHead(UserInfo *user_info_node) {
    ReqHead req_head;
    if (Recv(user_info_node->sock_fd, &req_head, sizeof(req_head), 0) == -1) {
        ERR("%s:%d: recv client req head error\n", __FILE__, __LINE__);
        return -1;
    }

    if (req_head.type != SEND_FILE) {
        ERR("%s:%d: recv client req head type != EEND_FILE error\n",
             __FILE__, __LINE__);
        return -1;
    }

    ReqFile req_file;
    if (Recv(user_info_node->sock_fd, &req_file, req_head.len, 0) == -1) {
        ERR("%s:%d: recv client file head error\n", __FILE__, __LINE__);
        return -1;
    }

    user_info_node->file_len = req_file.file_len;
    /* strncpy(user_info_node->send_file_name,        */
    /*         req_file.path, strlen(req_file.path)); */
    strcpy(user_info_node->send_file_name, "/home/cf/git/monitor/temp/t");

    return 0;
}

void *DoRecv(void *ptr) {
    int epollfd = epoll_create(EPOLL_SIZE);
    struct epoll_event event, events[EPOLL_SIZE];

    List *cur_list_node = NULL;
    UserInfo *cur_user_info = NULL;
    while (1) {
        for (cur_list_node = login_send;
                cur_list_node != NULL; cur_list_node = cur_list_node->next) {
            cur_user_info = (UserInfo *)cur_list_node->data;

            if (cur_user_info->is_start == STOP) {
                // 将客户端置为开始发送状态
                cur_user_info->is_start = START;

                event.data.ptr = cur_user_info;
                event.events = EPOLLIN;
                epoll_ctl(epollfd, EPOLL_CTL_ADD,
                          cur_user_info->sock_fd, &event);
            }
        }

        int nfds = epoll_wait(epollfd, events, EPOLL_SIZE, TIME_WAIT);
        int i;
        for (i = 0; i < nfds; ++i) {
            cur_user_info = (UserInfo *)events[i].data.ptr;

            if (cur_user_info->is_start == START) {
                int ret = 0;
                char buf[BUF_LEN] = { '\0' };
                switch (cur_user_info->send_flag) {
                    case SEND_HEAD:
                        if (RecvHead(cur_user_info) == -1) {
                            cur_user_info->send_flag = SEND_OVER;
                            break;
                        }

                        cur_user_info->file_fd =
                            open("/home/cf/git/monitor/temp/t",
                                 O_WRONLY | O_CREAT | O_TRUNC, 0664);
                            /* open(cur_user_info->send_file_name, */
                            /*      O_WRONLY | O_CREAT, 0664);     */
                        if (cur_user_info->file_fd < 0) {
                            ERR("%s:%d: create file error\n",
                                 __FILE__, __LINE__);
                            break;
                        }

                        cur_user_info->sfile_len = 0;
                        cur_user_info->send_flag = SEND_BODY;
                        break;
                    case SEND_BODY:
                        if (cur_user_info->sfile_len <
                                cur_user_info->file_len) {
                            memset(buf, '\0', BUF_LEN);
                            ret = Recv(cur_user_info->sock_fd, buf, BUF_LEN, 0);
                            printf("serv: recv %d byte\n", ret);
                            cur_user_info->sfile_len += ret;
                            if (ret < 0) {        // 接受客户端上传数据失败
                                cur_user_info->send_flag = SEND_OVER;
                                ERR("%s:%d: recv client file error\n",
                                     __FILE__, __LINE__);
                                break;
                            }
                            else if (ret == 0) {  // 客户端发送完毕（一个文件）
                                cur_user_info->send_flag = SEND_OVER;
                                break;
                            }

                            ret = Write(cur_user_info->file_fd, buf, ret);
                            printf("serv: save %d byte\n", ret);
                            if (ret < 0) {
                                ERR("%s:%d: save client file error\n",
                                     __FILE__, __LINE__);
                            }
                        }
                        else {
                            cur_user_info->send_flag = SEND_OVER;
                        }
                        break;
                    case SEND_OVER:
                        WriteOne(cur_user_info->busy_que,
                                 char, cur_user_info->send_file_name);
                        cur_user_info->send_flag = SEND_HEAD;
                        DBG("%s:%d: file:[%s] save success",
                             __FILE__, __LINE__,cur_user_info->send_file_name);

                        break;
                    default:
                        break;
                }
            }
        }
    }

    return ptr;
}

int StartRecvThread() {
    pthread_t tid = pthread_create(&tid, NULL, DoRecv, NULL);
    if (tid != 0) {
        ERR("%s:%d: create recv thread error:%s", strerror(errno),
             __FILE__, __LINE__);
        return -1;
    }

    return 0;
}
