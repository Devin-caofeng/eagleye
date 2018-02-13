#ifndef MYSQL_DB__
#define MYSQL_DB__

#include <mysql/mysql.h>
#include "./monitor.h"

int InsertUserToDB(ReqLogin *req_login);

int CheckUserInfoFromDB(ReqLogin *req_login);


#endif /* MYSQL_DB__ */
