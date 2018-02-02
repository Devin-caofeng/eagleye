#ifndef USER_H__
#define USER_H__


#include "queue.h"

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
    char dir[256];
    char name[18];
    char passwd[20];
    char send_file_name[256];

    int is_login;
    int is_start;
    int sock_fd;
    int file_fd;
    int send_flag;
    long long conn_time;

    Queue *free_que;
    Queue *busy_que;
} UserInfo;


UserInfo *AddUser();

#endif /* USER_H__ */
