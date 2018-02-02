#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/stat.h>

#include "../include/ctrl.h"


static struct Command head = { "", NULL };


static int addCommand(const char *name) {
    struct Command *new_command =
        (struct Command *)malloc(sizeof(struct Command));
    if (new_command == NULL) {
        return -1;
    }

    strncpy(new_command->name, name, strlen(name));
    new_command->next = NULL;
    head.next = new_command;

    return 0;
}

static int ctrl() {
    if (mkfifo("../ctrlinfo", 0664) < 0) {
        if (errno != EEXIST) {
            perror("mkfifo error");
            return -1;
        }
    }

    int command_fd = open("../ctrlinfo", O_RDONLY);
    if (command_fd < 0) {
        perror("open ctrlinfo error");
        return -1;
    }

    char command[COMMAND_LEN] = { '\0' };
    while (1) {
        memset(command, 0, sizeof(command));
        if (read(command_fd, command, sizeof(command)) > 0) {
            struct Command *cur = head.next;
            while (cur) {
                if (strncasecmp(command, cur->name,
                                strlen(cur->name) * sizeof(char)) == 0) {
                    printf("execute %s\n", cur->name);
                    return 0;
                }
                cur = cur->next;
            }
        }
        else {
            usleep(1000);
        }
    }

    return 0;
}

static void *doCtrl(void *args) {
    ctrl();
    return args;
}

int startCtrlThread() {
    addCommand("show");

    pthread_t tid;
    if (pthread_create(&tid, NULL, doCtrl, (void *)0) != 0) {
        perror("pthread create error");
        return -1;
    }

    return 0;
}
