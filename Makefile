DIR = ./src
CC = gcc
CFLAG = -Wall -g -I./include
LFLAG = -pthread -lzlog -lconfig -lmysqlclient -L./lib
SRC = $(wildcard $(DIR)/*.c)
OBJ = $(patsubst %.c, %.o, $(SRC))

BIN = ./serv ./cli

all:$(BIN)

$(OBJ):$(DIR)/%.o:$(DIR)/%.c
	$(CC) -o $@ -c $< $(CFLAG)

serv_need = $(DIR)/log.o $(DIR)/wrap.o $(DIR)/user.o $(DIR)/list.o $(DIR)/recv.o \
	$(DIR)/ctrl.o $(DIR)/send.o $(DIR)/scan_dir.o $(DIR)/monitor.o $(DIR)/mysql_db.o

cli_need = $(DIR)/cli.o $(DIR)/wrap.o

./serv:$(serv_need)
	$(CC) -o $@ $^ $(LFLAG)

./cli:$(cli_need)
	$(CC) -o $@ $^ $(LFLAG)


.PHONY:clean
clean:
	-rm -rf $(OBJ) $(BIN)
