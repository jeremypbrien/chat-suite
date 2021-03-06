#ifndef _SERVER_H
#define _SERVER_H

#include "message.h"

#include <stdbool.h>

typedef struct client_node_s {
  char *username;
  struct client_node_s *next_client;
  int fd;
  bool connected;
} client_node_t;

typedef struct queue_node_s {
  void *data;
  struct queue_node_s *next_task;
  int type;
} queue_node_t;

/* handlers */
void *connection_handler(void*);
void *queue_handler(void*);

/* helper functions */
void add_client(client_node_t*);
void remove_client(client_node_t*);
void add_task(int, void*);
void free_client_node(client_node_t *);
bool validate_message(client_node_t *, message_t*);
char *get_username(int);

void send_message(message_t *);


#endif /* _SERVER_H */
