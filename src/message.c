#include "message.h"

#include <stdlib.h>
#include <pthread.h>

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
