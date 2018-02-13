#include "../include/mysql_db.h"
#include "../include/log.h"
#include <string.h>

int InsertUserToDB(ReqLogin *req_login) {
    MYSQL *db_ptr = mysql_init(NULL);
    if (db_ptr == NULL) {
        ERR("db init error\n");
        return -1;
    }

    if (mysql_real_connect(db_ptr, "127.0.0.1", "root", "cf19960505",
                           NULL, 0, NULL, 0) == NULL) {
        ERR("connect mysql failed:[%s]\n", mysql_error(db_ptr));
        return -1;
    }

    if (mysql_select_db(db_ptr, "shar_stor_user") != 0) {
        ERR("select db error");
        return -1;
    }

    char sql_cmd[BUF_LEN];
    memset(sql_cmd, '\0', BUF_LEN);

    sprintf(sql_cmd, "insert into UserBaseInfo \
                      values('%s', '%s', '%s', NOW());",
            req_login->user_name, req_login->user_passwd,
            req_login->user_group);
    if (mysql_query(db_ptr, sql_cmd) != 0) {
        ERR("insert to db error");
        return -1;
    }

    return 0;
}

int CheckUserInfoFromDB(ReqLogin *req_login) {
    MYSQL *db_ptr = mysql_init(NULL);
    if (db_ptr == NULL) {
        ERR("db init error");
        return -1;
    }

    if (mysql_real_connect(db_ptr, "127.0.0.1", "root", "cf19960505",
                           NULL, 0, NULL, 0) == NULL) {
        ERR("connect db error");
        return -1;
    }

    if (mysql_select_db(db_ptr, "shar_stor_user") != 0) {
        ERR("select db error");
        return -1;
    }

    char sql_cmd[BUF_LEN];
    memset(sql_cmd, '\0', BUF_LEN);

    sprintf(sql_cmd, "select * from UserBaseInfo \
                      where name = '%s' and password = '%s';",
            req_login->user_name, req_login->user_passwd);
    if (mysql_query(db_ptr, sql_cmd) != 0) {
        ERR("select error");
        return -1;
    }

    MYSQL_RES *query_ret = mysql_store_result(db_ptr);
    if (query_ret == NULL) {
        ERR("store result error");
        return -1;
    }

    int num_rows = mysql_num_rows(query_ret);
    if (num_rows <= 0) {
        ERR("user info not exist");
        return -1;
    }

    return 0;
}
