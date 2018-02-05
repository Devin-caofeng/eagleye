#ifndef USER_H__
#define USER_H__


#include "queue.h"
#include "comm.h"

enum Status {
    START = 0,
    STOP
};

enum SendFlag {
    SEND_HEAD = 0,
    SEND_BODY,
    SEND_OVER
};

typedef struct {
    char dir[PATH_LEN];
    char name[NAME_LEN];
    char passwd[PASSWD_LEN];
    char send_file_name[FILE_NAME_LEN];
    int  file_len;
    int  sfile_len;

    int is_start;
    int sock_fd;
    int file_fd;
    int send_flag;
    long long conn_time;

    Queue *free_que;
    Queue *busy_que;
    char *ptr;
} UserInfo;


UserInfo *AddUser();

#endif /* USER_H__ */
