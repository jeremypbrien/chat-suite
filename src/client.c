#include "client.h"
#include "message.h"

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>


int main(int argc, char **argv) {
  
  /* Create socket to server*/
  int server_fd;
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    perror("Client socket creation failed");
    exit(EXIT_FAILURE);
  }

  /* Connect to server */
  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  server_addr.sin_port = htons(PORT);

  if (connect(server_fd, (struct sockaddr *) &server_addr, sizeof (server_addr)) < 0) {
    perror("Failed to connect to server");
    exit(EXIT_FAILURE);
  }
} /* main() */
