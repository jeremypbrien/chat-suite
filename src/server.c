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
  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY; 
  server_addr.sin_port = htons(PORT);
  if (bind(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
    perror("Failed to bind server socket");
    exit(EXIT_FAILURE);
  }

  /* Listen for connections */
  listen(server_fd, 5);

  /* Main connection loop */
  int client_fd;
  struct sockaddr_in client_addr;
  socklen_t client_len = sizeof(client_addr);
  while ((client_fd = accept(server_fd, (struct sockaddr*) &client_addr, &client_len))) {
    printf("Hello\n");
  } /* end while */ 

} /* main() */
