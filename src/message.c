#include "message.h"

#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

/*
 * Frees message struct
 */
void free_message(message_t *msg) {
  if (msg == NULL) {
    return;
  }

  if (msg->data != NULL) {
    free(msg->data);
  }
  free(msg);
} /* free_message() */


/*
 * Reads message from socket
 * Returns message and sets int pointer to return value
 */
message_t *receive_message(int fd, int *return_val) {
  /* Get header */
  uint8_t buffer[3] = {0};
  int ret = recv(fd, (void*) buffer, 3, 0);
  if (ret != 3) {
    *return_val = -1;
    return NULL;
  }

  /* Create msg and allocate buffer */
  message_t *msg = malloc(sizeof(message_t));
  assert(msg);
  msg->type = buffer[0];
  memcpy(&(msg->length), &(buffer[1]), 2);
  msg->length = ntohs(msg->length);
  msg->data = malloc(msg->length);
  assert(msg->data);

  /* Receive remaining data */
  int total = 0;
  while (total < msg->length) {
    ret = recv(fd, (void*) &((msg->data)[total]), msg->length - total, 0);
    if (ret < 0) { /* TODO check errno and handle accordingly */
      *return_val = -1;
      free_message(msg);
      return NULL;
    }
    total += ret;
  }
  *return_val = 0;
  return msg;
} /* receive_message() */
