#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <zlog.h>
#include "../include/log.h"

zlog_category_t *log_handle = NULL;

int OpenLog(const char *conf, const char *mode) {
    if (!conf) {
        fprintf(stdout, "log conf file is null\n");
        return -1;
    }
    // zlog初始化
    if (zlog_init(conf)) {
        fprintf(stdout, "zlog_init error\n");
        return -1;
    }
    // 获取日志操作句柄
    if ((log_handle = zlog_get_category(mode)) == NULL) {
        fprintf(stdout, "zlog_get_category error\n");
        return -1;
    }

    return 0;
}

void CloseLog() {
    // 关闭日志
    zlog_fini();
}
