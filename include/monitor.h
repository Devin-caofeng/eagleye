#ifndef MONITOR_H__
#define MONITOR_H__

#include <stdint.h>

#include "./list.h"
#include "./queue.h"

#define NAME_LEN    18
#define PASSWD_LEN  20
#define PATH_LEN    1024

// 定义描述命令类型的枚举
enum CmdType {
    REQ_LOGIN = 0,       // 请求登录
    RSP_LOGIN,           // 回复登录请求
    REQ_FILE,            // 请求文件
    RSP_FILE             // 回复文件请求
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
