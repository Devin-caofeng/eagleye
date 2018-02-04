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
#include "../include/user.h"
#include "../include/wrap.h"
#include "../include/queue.h"
#include "../include/monitor.h"


int SendHead(UserInfo *user_info_node) {
    char *file_name= NULL;
    if (!ReadOne(user_info_node->busy_que, char, file_name)) {
        ERR("queue have not file");
        return 0;
    }

    struct stat st;
    if (stat(file_name, &st) < 0) {
        ERR("get file:[%s] error", file_name);
        WriteOne(user_info_node->free_que, char, file_name);
        return -1;
    }

    ReqHead req_head;
    memset(&req_head, '\0', sizeof(req_head));
    req_head.type = REQ_FILE;
    req_head.len = sizeof(ReqFile);

    if (Send(user_info_node->sock_fd,
             (char *)&req_head, sizeof(req_head), 0) < 0) {
        ERR("send file common head error");
        WriteOne(user_info_node->free_que, char, file_name);
        return -1;
    }

    ReqFile req_file;
    memset(&req_file, '\0', sizeof(req_file));
    req_file.file_len = st.st_size;
    memcpy(req_file.path, file_name, strlen(file_name));

    if (Send(user_info_node->sock_fd,
             (char *)&req_file, req_head.len, 0) < 0) {
        ERR("send file:[%s] head error", req_file.path);
        WriteOne(user_info_node->free_que, char, file_name);
        return -1;
    }

    user_info_node->file_fd = open(file_name, O_RDONLY);
    if (user_info_node->file_fd < 0) {
        ERR("open file:[%s] error:%s", file_name, strerror(errno));
        WriteOne(user_info_node->free_que, char, file_name);
        return -1;
    }

    memset(user_info_node->send_file_name, '\0', 256);
    memcpy(user_info_node->send_file_name, file_name, strlen(file_name));
    WriteOne(user_info_node->free_que, char, file_name);
    user_info_node->send_flag = SEND_BODY;

    return 0;
}

void *DoSend(void *ptr) {
    int epollfd = epoll_create(EPOLL_SIZE);
    struct epoll_event events[EPOLL_SIZE], event;

    List *cur_list_node = NULL;
    UserInfo *cur_user_info = NULL;
    while (1) {
        for (cur_list_node = login_send;
                cur_list_node != NULL; cur_list_node = cur_list_node->next) {
            cur_user_info = (UserInfo *)cur_list_node->data;
            if (cur_user_info->is_start == STOP) {
                cur_user_info->is_start = START;
                event.data.ptr = cur_user_info;
                event.events = EPOLLIN;
                epoll_ctl(epollfd, EPOLL_CTL_ADD,
                          cur_user_info->sock_fd, &event);
            }
        }

        for (cur_list_node = login_send;
                cur_list_node != NULL; cur_list_node = cur_list_node->next) {
            cur_user_info = (UserInfo *)cur_list_node->data;
            if (cur_user_info->is_start == START) {
                struct stat st;
                stat(cur_user_info->send_file_name, &st);
                char *file_name = cur_user_info->send_file_name;

                char cmd[CMD_LEN];
                sprintf(cmd, "fdfs_upload_file /etc/fdfs/client.conf %s",
                        file_name);
                FILE *fp;
                /* int file_size = st.st_size; */
                /* int ret = 0;                */
                /* int send_len = 0;           */
                /* char buf[BUF_LEN];          */
                switch (cur_user_info->send_flag) {
                    case SEND_HEAD:
                        if (SendHead(cur_user_info) < 0) {
                            //
                        }
                        break;
                    case SEND_BODY:
                        fp = popen(cmd, "r");
                        char ret[BUF_LEN];
                        memset(ret, '\0', sizeof(ret));
                        fread(ret, sizeof(ret), 1, fp);
                        pclose(fp);

                        printf("%s--->", file_name);
                        printf("%s", ret);
                        cur_user_info->send_flag = SEND_OVER;

                        /* while (send_len < file_size) {                         */
                        /*     memset(buf, '\0', BUF_LEN);                        */
                        /*     ret = ReadN(cur_user_info->file_fd, buf, BUF_LEN); */
                        /*     if (ret == 0) {                                    */
                        /*         close(cur_user_info->file_fd);                 */
                        /*         cur_user_info->send_flag = SEND_OVER;          */
                        /*         break;                                         */
                        /*     }                                                  */
                        /*     if (Send(cur_user_info->sock_fd, buf, ret, 0)) {   */

                        /*     }                                                  */
                        /*     send_len += ret;                                   */
                        /* }                                                      */
                        break;
                    case SEND_OVER:
                        break;
                    default:
                        break;
                }
            }

        }

        int nfds = epoll_wait(epollfd, events, EPOLL_SIZE, TIME_WAIT);
        int i;
        for (i = 0; i < nfds; ++i) {
            cur_user_info = (UserInfo *)events[i].data.ptr;
            ReqHead rsp_head;
            if (Recv(cur_user_info->sock_fd,
                     (void *)&rsp_head, sizeof(ReqHead), 0) < 0) {
                //
            }
            if (rsp_head.type != RSP_FILE) {
                //
            }

            if (rsp_head.len != 0) {
                char buf[BUF_LEN];
                memset(buf, '\0', BUF_LEN);
                if (Recv(cur_user_info->sock_fd,
                         buf, rsp_head.len, 0) < 0) {
                    //
                    ERR("send file error:%s", buf);
                    cur_user_info->send_flag = SEND_HEAD;
                }
                continue;
            }

            DBG("file:[%s] send success!", cur_user_info->send_file_name);
            unlink(cur_user_info->send_file_name);
            cur_user_info->send_flag = SEND_HEAD;
        }
    }

    return ptr;
}

int StartSendThread() {
    pthread_t tid = pthread_create(&tid, NULL, DoSend, NULL);
    if (tid != 0) {
        ERR("create send shtread error:%s", strerror(errno));
        return -1;
    }

    return 0;
}
