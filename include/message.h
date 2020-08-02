#ifndef _MESSAGE_H
#define _MESSAGE_H

#include <stdint.h>

/* Message configs */
#define MAX_MESSAGE_SIZE 65535 /* max size of data */
#define PORT 8080

/* message type */
#define ERROR 0
#define INFO 1
#define LOGIN 2
#define DISCONNECT 3
#define MESSAGE 4


typedef struct message_s {
  uint8_t type; /* type of message */
  uint8_t *data; /* contents of message data */
  uint16_t length; /* length of data */
} message_t;

// Function declarations
message_t *parse_data(uint8_t*, int);
uint8_t *write_message(message_t*, int*);
void free_message(message_t*);
uint16_t end_swap_16(uint8_t[2]);

#endif /* _MESSAGE_H */
