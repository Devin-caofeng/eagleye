#ifndef MONITOR_H__
#define MONITOR_H__

#include "./list.h"
#include "./queue.h"
#include "comm.h"


// 定义描述命令类型的枚举
enum CmdType {
    REQ_LOGIN = 0,       // 请求登录
    RSP_LOGIN,           // 回复登录请求
    REQ_FILE,            // 请求文件
    RSP_FILE,            // 回复文件请求
    SEND_FILE,
    RECV_FILE_SUCC
};

// 请求头信息
typedef struct {
    char type;           // 命令类型
    long len;            // 请求数据长度
    char buff[0];        //
} ReqHead;

// 用户信息
typedef struct {
    char user_name[NAME_LEN];
    char user_passwd[PASSWD_LEN];
    char user_path[PATH_LEN];
} ReqLogin;

// 文件信息
typedef struct {
    char path[PATH_LEN];
    long file_len;
} ReqFile;


extern List *login_send;

#endif /* MONITOR_H__ */
