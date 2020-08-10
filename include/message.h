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


/*
 * MESSAGE DATA FORMAT
 * 1st byte: type of message
 * 2nd and 3rd byte: length of data following
 * All lengths are in network order (big endian) and unsigned
 * 
 * Types of data:
 * ERROR:      String containing error message
 * INFO:       String containing info message
 * LOGIN:      String containing username of client joining
 * DISCONNECT: String containing username of client disconnecting
 * MESSAGE: Length of username, username, length of message, message
 *          1 byte (n value), n bytes, 2 bytes (m value), m bytes
 */


typedef struct message_s {
  uint8_t type; /* type of message */
  uint8_t *data; /* contents of message data */
  uint16_t length; /* length of data */
} message_t;

// Function declarations
void free_message(message_t*);

#endif /* _MESSAGE_H */
