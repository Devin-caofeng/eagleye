#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <ctype.h>
#include "../include/wrap.h"

#define MAX_LINE      80
#define SERV_PORT     9000
#define BACK_LOG      66
#define INET_ADDR_LEN 120

typedef struct sockaddr SA;

int main() {

    int serv_fd = Socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serv_addr;
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    inet_pton(AF_INET, "192.168.223.133", &serv_addr.sin_addr);
    serv_addr.sin_port = htons(SERV_PORT);

    Connect(serv_fd, (SA *)&serv_addr, sizeof(serv_addr));

    char buf[MAX_LINE];
    while (fgets(buf, MAX_LINE, stdin) != NULL) {
        Write(serv_fd, buf, strlen(buf));
        // int n = Read(serv_fd, buf, MAX_LINE);
        int n = recv(serv_fd, buf, MAX_LINE, 0);

        if (n == 0) {
            printf("the other side has been closed\n");
            break;
        }
        else {
            Write(STDOUT_FILENO, buf, n);
        }
    }

    Close(serv_fd);

    return 0;
}
