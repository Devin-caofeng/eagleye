#ifndef CTRL_H__
#define CTRL_H__

#include "comm.h"


typedef struct command {
    char name[COMMAND_LEN];
    struct command *next;
} Command;

int startCtrlThread();

#endif /* CTRL_H__ */
