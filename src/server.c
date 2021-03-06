#include "server.h"
#include "message.h"

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <unistd.h>
#include <signal.h>
#include <stdint.h>
#include <errno.h>
#include <stdbool.h>


client_node_t *g_client_list_head = NULL;
queue_node_t *g_queue_head = NULL;

pthread_mutex_t g_queue_list_lock;

int main(int argc, char **argv) {
  signal(SIGPIPE, SIG_IGN);

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

  /* Initialize mutex locks */
  if (pthread_mutex_init(&g_queue_list_lock, NULL) != 0) {
    perror("Queue list lock failed");
    exit(EXIT_FAILURE);
  }

  /* Initialize queue handler */
  pthread_t queue_thread;
  assert(pthread_create(&queue_thread, NULL, queue_handler, NULL) == 0);

  /* Listen for connections */
  listen(server_fd, 5);

  /* Main connection loop */
  int fd;
  struct sockaddr_in client_addr;
  socklen_t client_len = sizeof(client_addr);
  while ((fd = accept(server_fd, (struct sockaddr*) &client_addr, &client_len))) {
    assert(fd > 0);
    pthread_t client_thread;

    /* Create pointer for client fd to be passed to thread */
    int *client_fd = malloc(sizeof(int));
    assert(client_fd);
    *client_fd = fd;

    /* Create client thread */
    //TODO HANDLE CONNECTION ERROR
    assert(pthread_create(&client_thread, NULL, connection_handler, (void*) client_fd) == 0);
  } /* end while */ 
} /* main() */


/*
 * Adds client to list of connected clients
 */
void add_client(client_node_t *node) {
  DEBUG_PRINT("Adding client: %d\n", node->fd);
  if (g_client_list_head == NULL) {
    g_client_list_head = node;
    return;
  }

  client_node_t *current_node = g_client_list_head;
  while (current_node->next_client != NULL) {
    current_node = current_node->next_client;
  }
  current_node->next_client = node;
} /* add_client() */


/*
 * Removes client from list of connected clients
 * Client node is freed by queue_handler
 */
void remove_client(client_node_t *node) {
  DEBUG_PRINT("Removing client: %d\n", node->fd);
  if (g_client_list_head == node) {
    g_client_list_head = node->next_client;
    return;
  }
  client_node_t *current_node = g_client_list_head;
  while (current_node->next_client != node) {
    current_node = current_node->next_client;
  }
  current_node->next_client = node->next_client;
}


/*
 * Handler for each client thread. Adds tasks to queue based on client actions.
 * Besides the initial client_fd buffer, all memory is freed in the queue handler.
 */
void *connection_handler(void *data) {
  /* get client fd and free buffer */
  int client_fd = *((int*) data);
  free(data);

  /* Construct client object for list 
   * NOTE: eventually freed by queue_handler
   */
  client_node_t *client_node = malloc(sizeof(client_node_t));
  assert(client_node);
  client_node->next_client = NULL;
  client_node->fd = client_fd;
  client_node->connected = true;

  client_node->username = get_username(client_node->fd);
  if (client_node->username == NULL) {
    DEBUG_PRINT("Client <%d> failed username setup\n", client_node->fd);
    shutdown(client_node->fd, SHUT_RDWR);
    free_client_node(client_node);
    return NULL;
  }

  /* Create task to add user */
  add_task(LOGIN, (void*) client_node);

  /* mainloop for client */
  while (1) {
    int ret;
    message_t *msg = receive_message(client_node->fd, &ret);
    
    /* Check return and message validity */
    if ((ret != 0) || (!(validate_message(client_node, msg)))) {
      client_node->connected = false; /* alert send_message of disconnect */
      break;
    }

    add_task(MESSAGE, (void*) msg);
  } /* end mainloop */
  DEBUG_PRINT("Client <%d> connection closed\n", client_node->fd);

  /* Handle client disconnect */
  add_task(DISCONNECT, (void*) client_node);
  /* no need to free since handled by queue_handler */

  return NULL;
} /* connection_handler() */


/*
 * Performs tasks in queue and frees memory after sending to clients
 */
void *queue_handler(void *unused) {
  while (1) {
    /* waits until queue has task */
    while (g_queue_head == NULL) {
    }

    /* remove first task from queue */
    pthread_mutex_lock(&g_queue_list_lock);
    queue_node_t *task = g_queue_head;
    g_queue_head = task->next_task;
    pthread_mutex_unlock(&g_queue_list_lock);

    /* Perform task */
    if (task->type == LOGIN) {
      /* Get client node and add to list of clients */
      client_node_t *node = (client_node_t*) task->data;
      add_client(node);

      /* Create msg and send to clients */
      message_t *msg = malloc(sizeof(message_t));
      assert(msg);
      msg->type = LOGIN;
      msg->length = strlen(node->username) + 1;
      msg->data = malloc(msg->length);
      assert(msg->data);
      strncpy((char*) msg->data, node->username, msg->length + 1);
      send_message(msg);

      /* Free memory 
       * We don't want to free node since part of linked list
       */
      free_message(msg);
      free(task);
    }
    else if (task->type == DISCONNECT) {
      /* Get client node and remove list of clients */
      client_node_t *node = (client_node_t*) task->data;
      remove_client(node);

      /* Alert client of disconnect */
      shutdown(node->fd, SHUT_RDWR);

      /* Create msg and send to clients */
      message_t *msg = malloc(sizeof(message_t));
      assert(msg);
      msg->type = DISCONNECT;
      msg->length = strlen(node->username) + 1;
      msg->data = malloc(msg->length);
      assert(msg->data);
      strncpy((char*) msg->data, node->username, msg->length);
      send_message(msg);

      /* Free memory */
      free_message(msg);
      free(task);
      free_client_node(node);
    } 
    else {
      /* Get message and send to clients */
      message_t *msg = (message_t*) task->data;
      send_message(msg);

      /* Free memory */
      free_message(msg);
      free(task);
    }
  } /* end while */
  return NULL;
} /* queue_handler() */


void send_message(message_t *msg) {
  DEBUG_PRINT("Echoing msg with type <%d> to clients\n", msg->type);
  /* Create buffer to send data */
  int length = msg->length + 3;
  uint8_t *buffer = malloc(length);
  assert(buffer);
  
  /* Fill buffer */
  uint16_t network_length = htons(msg->length);
  buffer[0] = msg->type;
  memcpy(&buffer[1], &network_length, 2);
  memcpy(&buffer[3], msg->data, msg->length);
  
  /* Send to each client */
  client_node_t *node = g_client_list_head;
  while (node != NULL) {
    if (node->connected) {
      send(node->fd, (void*) buffer, length, 0);
      //TODO MAKE SURE ALL DATA IS SENT SEND() DOES NOT ALWAYS WORK
      if ((errno == EPIPE) || (errno == ECONNRESET)) {
        node->connected = false;
      }
    }
    node = node->next_client;
  }
  free(buffer);
} /* send_message() */


/*
 * Adds task to end of queue
 */
void add_task(int type, void *data) {
  pthread_mutex_lock(&g_queue_list_lock);

  /* Create node */
  queue_node_t *node = malloc(sizeof(queue_node_t));
  assert(node);
  node->type = type;
  node->data = data;
  node->next_task = NULL;

  /* Add node to queue */
  if (g_queue_head == NULL) {
    g_queue_head = node;
    pthread_mutex_unlock(&g_queue_list_lock);
    return;
  }

  queue_node_t *current_node = g_queue_head;
  while (current_node->next_task != NULL) {
    current_node = current_node->next_task;
  }
  current_node->next_task = node;
  DEBUG_PRINT("Added task with type <%d>\n", node->type);
  pthread_mutex_unlock(&g_queue_list_lock);

} /* add_task() */


bool validate_message(client_node_t *node, message_t *msg) {
  assert(node);
  assert(msg);

  if (msg->type == MESSAGE) {
    int username_size = msg->data[0];
    char username[username_size];
    strncpy(username, (char*) &((msg->data)[1]), username_size);
    if (strncmp(username, node->username, username_size) != 0) {
      return false;
    }
  }
  return true;
} /* validate_message() */

/*
 * Frees node after removal from list
 */
void free_client_node(client_node_t *node) {
  if (node != NULL) {
    free(node->username);
    free(node);
  }
} /* free_client_node() */


char *get_username(int fd) {
  uint8_t size = 0;
  int ret = recv(fd, &size, 1, 0);
  if (ret != 1) {
    return NULL;
  }
  char *username = malloc(size);
  assert(username);
  ret = recv(fd, username, size, 0);
  if (ret != size) {
    free(username);
    return NULL;
  }
  assert(username[size - 1] == '\0');
  return username;
} /* get_username() */
