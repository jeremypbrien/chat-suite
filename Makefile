SRC_DIR := src
OBJ_DIR := obj

CC := gcc

GTK_CFLAGS = `pkg-config --cflags gtk+-3.0`
GTK_LIBS = -lm `pkg-config --libs gtk+-3.0`

CPPFLAGS := -Iinclude -D _GNU_SOURCE -D DEBUG
CFLAGS   := -g -Wall
LDFLAGS  := -Llib
LDLIBS   := -lpthread


.PHONY: all install clean update-tags

all: clean client server update-tags

install: client server

update-tags:
	ctags -R

client: message
	$(CC) $(CPPFLAGS) $(CFLAGS) $(GTK_CFLAGS) $(LDFLAGS) $(LDLIBS) $(GTK_LIBS) $(SRC_DIR)/client.c $(OBJ_DIR)/message.o -o client

server: message
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) $(LDLIBS) $(SRC_DIR)/server.c $(OBJ_DIR)/message.o -o server

message: $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $(SRC_DIR)/message.c -o $(OBJ_DIR)/message.o

$(OBJ_DIR):
	mkdir $@

clean:
	$(RM) client server obj/*
