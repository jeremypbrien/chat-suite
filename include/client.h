#ifndef _CLIENT_H
#define _CLIENT_H

#include "message.h"

#include <gtk/gtk.h>

typedef struct queue_node_s {
  message_t *msg;
  struct queue_node_s *next;
} queue_node_t;

int establish_connection();
void *recv_handler(void*);
void *send_handler(void*);
void add_msg_to_queue(GtkEntry*, gpointer);
void display_message(message_t*);
void activate(GtkApplication*, gpointer);
void send_username(char*);

#endif /* _CLIENT_H */
