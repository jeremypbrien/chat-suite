SRC_DIR := src
OBJ_DIR := obj

CC := gcc

CPPFLAGS := -Iinclude
CFLAGS   := -g -Wall -std=c99
LDFLAGS  := -Llib
LDLIBS   := -lpthread


.PHONY: all install clean update-tags

all: clean client server update-tags

install: client server

update-tags:
	ctags -R

client: message
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SRC_DIR)/client.c $(OBJ_DIR)/message.o -o client

server: message
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SRC_DIR)/server.c $(OBJ_DIR)/message.o -o server

message: $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $(SRC_DIR)/message.c -o $(OBJ_DIR)/message.o

$(OBJ_DIR):
	mkdir $@

clean:
	$(RM) client server obj/*
