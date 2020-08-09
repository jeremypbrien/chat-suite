#include "message.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <arpa/inet.h>


/*
 * Reads data from socket recv buffer and parses into message struct
 */
message_t *parse_data(uint8_t *buffer, int len) {
  assert(buffer);
  assert(len > 3);

  message_t *msg = malloc(sizeof(message_t));
  assert(msg);

  msg->type = buffer[0];
  memcpy(&(msg->length), &buffer[1], 2);
  msg->length = ntohs(msg->length); /* convert to host order */
  
  msg->data = malloc(len - 3);
  assert(msg->data);
  memcpy(msg->data, &buffer[3], len - 3);

  return msg;
} /* parse_data() */


/*
 * Writes message struct to buffer for socket
 * Length of buffer is stored in int pointer
 */
uint8_t *write_message(message_t *msg, int *len) {
  assert(msg);
  assert(len);

  *len = 3 + msg->length;
  uint8_t *buffer = malloc(3 + msg->length);
  assert(buffer);

  buffer[0] = msg->type;

  msg->length = htons(msg->length); /* convert to network order */
  memcpy(&buffer[1], &(msg->length), 2);
  memcpy(&buffer[3], msg->data, msg->length);
  
  return buffer;
} /* write_message() */


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
 * Swap endianess for 16 bit value
 */
uint16_t end_swap_16(uint8_t value[2]) {
  union {
    uint16_t new_int;
    uint8_t int_array[2];
  } int_union;

  int_union.int_array[0] = value[1];
  int_union.int_array[1] = value[0];
  return int_union.new_int;
} /* end_swap_16() */
