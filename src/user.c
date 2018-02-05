#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/user.h"

#define BLOCK_SIZE 256

UserInfo *AddUser() {
    UserInfo *new_user = (UserInfo *)malloc(sizeof(UserInfo));
    memset(new_user, '\0', sizeof(UserInfo));
    new_user->is_start = STOP;

    InitQueue(new_user->free_que, char, 100);
    InitQueue(new_user->busy_que, char, 100);

    new_user->is_start = STOP;
    new_user->send_flag = SEND_HEAD;
    new_user->sfile_len = 0;
    new_user->file_fd = -1;

    new_user->ptr = (char *)malloc(BLOCK_SIZE * 100);
    int i = 0;
    for (i = 0; i < 100; ++i) {
        WriteOne(new_user->free_que, char, new_user->ptr + i * BLOCK_SIZE);
    }

    strcpy(new_user->dir, "/home/cf/git/monitor/log");

    return new_user;
}
