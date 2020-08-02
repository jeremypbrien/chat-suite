#include "server.h"
#include "message.h"

#include <netinet/in.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>


int main(int argc, char **argv) {

  /* Create server socket */
  int server_fd;
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    perror("Server socket creation failed");
    exit(EXIT_FAILURE);
  }
  
  /* Bind server socket */
  struct sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY; 
  address.sin_port = htons(PORT);
  if (bind(server_fd, (struct sockaddr *) &address, sizeof(address)) < 0) {
    perror("Failed to bind server socket");
    exit(EXIT_FAILURE);
  }

  /* Listen for connections */
  listen(server_fd, 5);

} /* main() */
