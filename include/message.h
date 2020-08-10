#ifndef _MESSAGE_H
#define _MESSAGE_H

#include <stdint.h>
#include <stdio.h>

/* Debug print */
#ifdef DEBUG
#define DEBUG_PRINT(...) do { fprintf(stderr, __VA_ARGS__ ); } while(0)
#else
#define DEBUG_PRINT(...) do {} while (0)
#endif

/* Message configs */
#define MAX_MESSAGE_SIZE 65535 /* max size of data */
#define PORT 8080

/* message type */
#define MESSAGE 0
#define LOGIN 1
#define DISCONNECT 2
#define INFO 3
#define ERROR 4


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
