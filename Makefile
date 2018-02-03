DIR = ./src
CC = gcc
CFLAG = -Wall -g -I./include
LFLAG = -pthread -lzlog -lconfig -L./lib
SRC = $(wildcard $(DIR)/*.c)
OBJ = $(patsubst %.c, %.o, $(SRC))

# lmydynmic = ./bin/libmydynmic.so
BIN = ./serv ./cli

all:$(BIN)

$(OBJ):$(DIR)/%.o:$(DIR)/%.c
	$(CC) -o $@ -c $< $(CFLAG)

eagleye_need = $(DIR)/log.o $(DIR)/wrap.o $(DIR)/user.o $(DIR)/list.o \
			   $(DIR)/ctrl.o $(DIR)/send.o $(DIR)/scan_dir.o $(DIR)/monitor.o

cli_need = $(DIR)/cli.o $(DIR)/wrap.o

./serv:$(eagleye_need)
	$(CC) -o $@ $^ $(LFLAG)

./cli:$(cli_need)
	$(CC) -o $@ $^ $(LFLAG)

.PHONY:clean
clean:
	-rm -rf $(OBJ) $(BIN) $(all)
