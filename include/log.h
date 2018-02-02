#ifndef LOG_H__
#define LOG_H__

#include <zlog.h>

extern zlog_category_t *log_handle;

#define ALL(...) zlog_fatal(log_handle, __VA_ARGS__)
#define INF(...) zlog_info(log_handle, __VA_ARGS__)
#define WAR(...) zlog_warn(log_handle, __VA_ARGS__)
#define DBG(...) zlog_debug(log_handle, __VA_ARGS__)
#define ERR(...) zlog_error(log_handle, __VA_ARGS__)

int OpenLog(const char *conf, const char *mode);
void CloseLog();

#endif /* LOG_H__ */
