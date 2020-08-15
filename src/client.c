#include "client.h"
#include "message.h"

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <gtk/gtk.h>
#include <glib.h>

queue_node_t *g_queue_head = NULL;
pthread_mutex_t g_queue_list_lock;
int g_socket_fd = -1;
char *g_username = NULL;
GtkWidget *g_chat_window = NULL;

int main(int argc, char **argv) {
  /* Initialize mutex locks */
  if (pthread_mutex_init(&g_queue_list_lock, NULL) != 0) {
    perror("Queue list lock failed");
    exit(EXIT_FAILURE);
  }

  // TODO temp username for testing
  g_username = "Bob";

  /* Launch GUI */
  GtkApplication *app = gtk_application_new("org.jeremypbrien", G_APPLICATION_NON_UNIQUE);
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
  int status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);
  return status;
} /* main() */


int establish_connection() {
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
  return server_fd;
} /* establish_connection() */


void *recv_handler(void *unused) {
  // NOTE the length of the message text is in network order
  while (1) {
    int ret;
    message_t *msg = receive_message(g_socket_fd, &ret);

    if (ret != 0) {
      g_socket_fd = -1;
      break;
    }
    DEBUG_PRINT("Received message of type <%d>\n", msg->type);
    display_message(msg);
    free_message(msg);
  }
  DEBUG_PRINT("WARNING: Exiting %s\n", __func__);
  return NULL;
} /* recv_handler() */


void display_message(message_t *msg) {
  GtkTextBuffer *text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(g_chat_window));
  GtkTextIter iter;
  gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(text_buffer), &iter);

  char *txt = NULL; /* must be null terminated */
  if (msg->type == MESSAGE) {
    /* Get username */
    int username_size = (msg->data)[0];
    char username[username_size];
    strncpy(username, (char*) &((msg->data)[1]), username_size);

    /* Get text of message */
    uint16_t msg_text_size;
    memcpy(&msg_text_size, &((msg->data)[1 + username_size]), 2);
    msg_text_size = ntohs(msg_text_size);
    char msg_text[msg_text_size];
    strncpy(msg_text, (char*) &((msg->data)[3 + username_size]), msg_text_size);

    asprintf(&txt, "%s: %s\n", username, msg_text);
  }
  else if (msg->type == LOGIN) {
    asprintf(&txt, "%s connected.\n", msg->data);
  }
  else if (msg->type == DISCONNECT) {
    asprintf(&txt, "%s disconnected.\n", msg->data);
  }
  /* TODO support for ERROR and INFO */
  assert(txt);
  gtk_text_buffer_insert(GTK_TEXT_BUFFER(text_buffer), &iter, txt, -1);
  free(txt);

} /* display_message() */


void *send_handler(void *unused) {
  while (1) {
    /* Wait for work */
    while (g_queue_head == NULL) {
    }
    DEBUG_PRINT("Sending message to server\n");

    /* Pop top node */
    pthread_mutex_lock(&g_queue_list_lock);
    queue_node_t *node = g_queue_head;
    g_queue_head = g_queue_head->next;
    pthread_mutex_unlock(&g_queue_list_lock);
    
    /* Create and fill buffer */
    uint8_t *buffer = malloc(3 + node->msg->length);
    uint16_t network_length = htons(node->msg->length);
    assert(buffer);
    buffer[0] = node->msg->type;
    memcpy(&(buffer[1]), &network_length, 2);
    memcpy(&(buffer[3]), node->msg->data, node->msg->length);

    /* Send buffer */
    send(g_socket_fd, (void*) buffer, 3 + node->msg->length, 0);

    /* Free memory */
    free(buffer);
    free_message(node->msg);
    free(node);
  }
} /* send_handler() */


void add_msg_to_queue(GtkEntry *entry, gpointer unused) {
  DEBUG_PRINT("Adding message to queue\n");
  /* get text of entry */
  const char *widget_text = gtk_entry_get_text(GTK_ENTRY(entry));
  char *text = malloc(strlen(widget_text) + 1);
  assert(text);
  strncpy(text, widget_text, strlen(widget_text) + 1);
  gtk_entry_set_text(GTK_ENTRY(entry), "");

  /* Create message */
  message_t *msg = malloc(sizeof(message_t));
  assert(msg);
  msg->type = MESSAGE;

  /* Create buffer */
  uint8_t username_size = strlen(g_username) + 1;
  uint16_t text_size = strlen(text) + 1;
  uint16_t network_text_size = htons(text_size);
  assert(1 + username_size + 2 + text_size <= 65535); // TODO make this never fail
  // 1 is for username size, 2 is for text size in buffer
  msg->length = 1 + username_size + 2 + text_size;
  msg->data = malloc(msg->length);
  assert(msg->data);

  /* Fill buffer */
  int index = 0;
  (msg->data)[index++] = username_size;
  strncpy((char*) &((msg->data)[index]), g_username, username_size);
  index += username_size;
  memcpy(&((msg->data)[index]), &network_text_size, 2);
  index += 2;
  strncpy((char*) &((msg->data)[index]), text, text_size);
  assert((index + text_size) == msg->length);

  /* Create node */
  queue_node_t *node = malloc(sizeof(queue_node_t));
  assert(node);
  node->msg = msg;
  node->next = NULL;
  
  /* Add to queue */
  pthread_mutex_lock(&g_queue_list_lock);
  if (g_queue_head == NULL) {
    g_queue_head = node;
    pthread_mutex_unlock(&g_queue_list_lock);
    return;
  }

  queue_node_t *current = g_queue_head;
  while (current->next != NULL) {
    current = current->next;
  }
  current->next = node;
  pthread_mutex_unlock(&g_queue_list_lock);
} /* add_msg_to_queue() */


void activate(GtkApplication *app, gpointer unused) {
  /* Window */
  GtkWidget *window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(window), "Chat Client");

  /* Container */
  GtkWidget *main_ctr = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_container_add(GTK_CONTAINER(window), main_ctr);

  /* Chat window */
  GtkWidget *chat_scroll_bar = gtk_scrolled_window_new(NULL, NULL);
  g_chat_window = gtk_text_view_new();
  gtk_container_add(GTK_CONTAINER(chat_scroll_bar), g_chat_window);
  gtk_box_pack_start(GTK_BOX(main_ctr), chat_scroll_bar, TRUE, TRUE, 15);

  /* Text entry */
  GtkWidget *text_entry = gtk_entry_new();
  gtk_box_pack_start(GTK_BOX(main_ctr), text_entry, FALSE, TRUE, 5);
  g_signal_connect(text_entry, "activate", G_CALLBACK(add_msg_to_queue), NULL);

  gtk_widget_show_all(window);

  /* Connection */
  g_socket_fd = establish_connection();
  send_username(g_username);

  /* Start threads */
  pthread_t send_thread;
  pthread_t recv_thread;
  assert(pthread_create(&send_thread, NULL, send_handler, NULL) == 0);
  assert(pthread_create(&recv_thread, NULL, recv_handler, NULL) == 0);
} /* activate() */


void send_username(char *username) {
  uint8_t size = strlen(username) + 1;
  send(g_socket_fd, &size, 1, 0);
  send(g_socket_fd, username, size, 0);

}
