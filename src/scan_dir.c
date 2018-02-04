#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <dirent.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>

#include "../include/log.h"
#include "../include/comm.h"
#include "../include/list.h"
#include "../include/user.h"
#include "../include/queue.h"
#include "../include/monitor.h"
#include "../include/scan_dir.h"


static int CheckIsUse(const char *file) {
    if (file == NULL) return -1;

    char cmd[CMD_LEN];
    sprintf(cmd, "fuser -a %s 2>&1", file);

    FILE *fp = popen(cmd, "r");
    char ret[BUF_LEN];
    memset(ret, '\0', sizeof(ret));
    fread(ret, sizeof(ret), 1, fp);

    pclose(fp);

    if (ret[strlen(ret) - 2] != ':') return 0;
    else return -1;
}

static int ScanDir(const char *path, UserInfo *head) {
    if (path == NULL) return -1;

    // 工作队列中还有数据未发送，直接返回
    if (CanRead(head->busy_que)) return 0;

    DIR *dir = opendir(path);
    if (dir == NULL) {
        printf("opendir : [%s] error %s\n", path, strerror(errno));
        return -1;
    }

    struct dirent *dir_ent = NULL;
    while ((dir_ent = readdir(dir)) != NULL) {
        if (!strcmp(dir_ent->d_name, ".") || !strcmp(dir_ent->d_name, "..")) {
            continue;
        }

        char buf[PATH_LEN] = { '\0' };
        snprintf(buf, sizeof(buf) - 1, "%s/%s", path, dir_ent->d_name);
        if (dir_ent->d_type == 4) {
            ScanDir(buf, head);
            continue;
        }

        if (CheckIsUse(buf) != 0) {
            char *ptr = NULL;
            if (!ReadOne(head->free_que, char, ptr)) {
                closedir(dir);
                return 0;
            }
            DBG("input file:[%s] to busy queue!\n", buf);
            memset(ptr, '\0', 256);
            memcpy(ptr, buf, strlen(buf));
            WriteOne(head->busy_que, char, ptr);
        }
    }

    closedir(dir);

    return 0;
}

static void *DoScanDir(void *ptr) {
    List *cur_list = NULL;
    UserInfo *cur_user_info = NULL;

    while (1) {
        for (cur_list = login_send;
                cur_list != NULL; cur_list = cur_list->next) {
            cur_user_info = (UserInfo *)cur_list->data;
            ScanDir(cur_user_info->dir, cur_user_info);
//            ScanDir("/home/cf/git/monitor/log", cur_user_info);
            sleep(1);
        }

    }

    return ptr;
}

int StartScanDirThread(const char *path) {
    pthread_t tid = pthread_create(&tid, NULL, DoScanDir, (void *)path);
    if (tid != 0) {
        perror("thread create error");
        return -1;
    }

    return 0;
}
