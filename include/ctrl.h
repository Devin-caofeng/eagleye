#ifndef __CTRL_H__
#define __CTRL_H__

#define COMMAND_LEN 66

struct Command {
    char name[COMMAND_LEN];
    struct Command *next;
};

int startCtrlThread();

#endif /* __CTRL_H__ */
