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
#include <gtk/gtk.h>
#include <glib.h>

int main(int argc, char **argv) {
  setvbuf(stdout, NULL, _IONBF, 0);
  setvbuf(stderr, NULL, _IONBF, 0);
  setvbuf(stdin, NULL, _IONBF, 0);
  GtkApplication *app = gtk_application_new("jeremypbrien.chat-suite", G_APPLICATION_FLAGS_NONE);
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
  int status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);
  return status;
} /* main() */


void setup_connection() {
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
} /* setup_connection() */


void activate(GtkApplication *app, gpointer unused) {
  setup_connection();

  GtkWidget *window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(window), "Chat Client");

  gtk_widget_show_all(window);
} /* activate() */
