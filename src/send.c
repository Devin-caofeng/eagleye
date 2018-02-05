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


int GetSendFileName(UserInfo *user_info_node, char *file_name) {
    char *ptr = NULL;
    if (!ReadOne(user_info_node->busy_que, char, ptr)) return 1;

    strncpy(file_name, ptr, strlen(ptr));

    printf("%s:%d: file name [%s]\n", __FILE__, __LINE__, file_name);

    if (access(file_name, F_OK) < 0) {
        ERR("%s:%d: file:[%s] not exist", __FILE__, __LINE__, file_name);
        WriteOne(user_info_node->free_que, char, file_name);
        return -1;
    }

    return 0;
}

void *DoSend(void *ptr) {

    List *cur_list_node = NULL;
    UserInfo *cur_user_info = NULL;

    while (1) {
        for (cur_list_node = login_send;
                cur_list_node != NULL; cur_list_node = cur_list_node->next) {
            cur_user_info = (UserInfo *)cur_list_node->data;

            char file_name[PATH_LEN];
            memset(file_name, '\0', PATH_LEN);
            if (GetSendFileName(cur_user_info, file_name) != 0) {
                continue;
            }

            char cmd[CMD_LEN];
            sprintf(cmd, "fdfs_upload_file /etc/fdfs/client.conf %s",
                    file_name);
            printf("%s:%d: file name [%s]\n", __FILE__, __LINE__, file_name);

            FILE *fp = popen(cmd, "r");
            char ret[BUF_LEN];
            memset(ret, '\0', sizeof(ret));
            fread(ret, sizeof(ret), 1, fp);
            pclose(fp);

            printf("%s--->%s", file_name, ret);

            DBG("file:[%s] send success!", cur_user_info->send_file_name);
            unlink(file_name);
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
