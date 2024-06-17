#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

extern char last_method[16];
extern char last_content[BUFFER_SIZE];
extern char last_path[256];

void send_confirmation_page(int client_socket);
void send_file(int client_socket, const char *file_path, const char *content_type);
void handle_client(int client_socket);

#endif // SERVER_H