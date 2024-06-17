#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

char last_method[16];
char last_content[BUFFER_SIZE];
char last_path[256];

void send_confirmation_page(int client_socket) {
    FILE *html_file = fopen("confirm.html", "r");
    if (html_file == NULL) {
        perror("fopen failed");
        return;
    }

    fseek(html_file, 0, SEEK_END);
    long html_length = ftell(html_file);
    fseek(html_file, 0, SEEK_SET);
    char *html_content = malloc(html_length + 1);
    fread(html_content, 1, html_length, html_file);
    html_content[html_length] = '\0';
    fclose(html_file);

    // Replace {{PATH}} with the original path
    char *path_placeholder = strstr(html_content, "{{PATH}}");
    if (path_placeholder) {
        char *new_html_content = malloc(strlen(html_content) + strlen(last_path) - strlen("{{PATH}}") + 1);
        strncpy(new_html_content, html_content, path_placeholder - html_content);
        new_html_content[path_placeholder - html_content] = '\0';
        strcat(new_html_content, last_path);
        strcat(new_html_content, path_placeholder + strlen("{{PATH}}"));
        free(html_content);
        html_content = new_html_content;
    }

    // Replace {{METHOD}} with actual HTTP method
    char *method_placeholder = strstr(html_content, "{{METHOD}}");
    if (method_placeholder) {
        char *new_html_content = malloc(strlen(html_content) + strlen(last_method) - strlen("{{METHOD}}") + 1);
        strncpy(new_html_content, html_content, method_placeholder - html_content);
        new_html_content[method_placeholder - html_content] = '\0';
        strcat(new_html_content, last_method);
        strcat(new_html_content, method_placeholder + strlen("{{METHOD}}"));
        free(html_content);
        html_content = new_html_content;
    }

    // Replace {{CONTENT}} with actual content
    char *content_placeholder = strstr(html_content, "{{CONTENT}}");
    if (content_placeholder) {
        char *new_html_content = malloc(strlen(html_content) + strlen(last_content) - strlen("{{CONTENT}}") + 1);
        strncpy(new_html_content, html_content, content_placeholder - html_content);
        new_html_content[content_placeholder - html_content] = '\0';
        strcat(new_html_content, last_content);
        strcat(new_html_content, content_placeholder + strlen("{{CONTENT}}"));
        free(html_content);
        html_content = new_html_content;
    }

    char response[BUFFER_SIZE];
    snprintf(response, sizeof(response),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: text/html\r\n"
             "Content-Length: %ld\r\n"
             "\r\n",
             strlen(html_content));

    // Send the response headers
    send(client_socket, response, strlen(response), 0);

    // Send the HTML content
    send(client_socket, html_content, strlen(html_content), 0);

    free(html_content);
}

void send_file(int client_socket, const char *file_path, const char *content_type) {
    FILE *file = fopen(file_path, "r");
    if (file == NULL) {
        perror("fopen failed");
        return;
    }

    fseek(file, 0, SEEK_END);
    long file_length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *file_content = malloc(file_length + 1);
    fread(file_content, 1, file_length, file);
    file_content[file_length] = '\0';
    fclose(file);

    char response[BUFFER_SIZE];
    snprintf(response, sizeof(response),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: %s\r\n"
             "Content-Length: %ld\r\n"
             "\r\n",
             content_type, file_length);

    // Send the response headers
    send(client_socket, response, strlen(response), 0);

    // Send the file content
    send(client_socket, file_content, file_length, 0);

    free(file_content);
}

void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    int read_size;

    // Read the client's request
    read_size = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    if (read_size < 0) {
        perror("recv failed");
        return;
    }
    buffer[read_size] = '\0';

    // Log the received request
    printf("Received request:\n%s\n", buffer);

    // Parse the request line
    char method[16], path[256], version[16];
    sscanf(buffer, "%15s %255s %15s", method, path, version);

    printf("Parsed method: %s\n", method);
    printf("Parsed path: %s\n", path);
    printf("Parsed version: %s\n", version);

    char response[BUFFER_SIZE];

    // Handle the GET request for the root path
    if (strcmp(method, "GET") == 0 && strcmp(path, "/") == 0) {
        send_file(client_socket, "index.html", "text/html");
    } else if (strcmp(method, "GET") == 0 && strcmp(path, "/confirm") == 0) {
        send_confirmation_page(client_socket);
    } else if (strcmp(method, "GET") == 0 && strcmp(path, "/scripts.js") == 0) {
        send_file(client_socket, "scripts.js", "application/javascript");
    } else if ((strcmp(method, "POST") == 0 && strncmp(path, "/post", 5) == 0) ||
               (strcmp(method, "PUT") == 0 && strncmp(path, "/put", 4) == 0) ||
               (strcmp(method, "DELETE") == 0 && strncmp(path, "/delete", 7) == 0)) {
        // Get the content of the POST/PUT/DELETE request
        char *content = strstr(buffer, "\r\n\r\n");
        if (content) {
            content += 4; // Move past "\r\n\r\n"
        } else {
            content = "";
        }

        // Read additional content if any
        int content_length = 0;
        char *content_length_str = strstr(buffer, "Content-Length:");
        if (content_length_str) {
            sscanf(content_length_str, "Content-Length: %d", &content_length);
            if (content_length > 0) {
                char *body_start = strstr(content, "\r\n\r\n");
                if (body_start) {
                    body_start += 4; // Move past "\r\n\r\n"
                    strncpy(last_content, body_start, content_length);
                    last_content[content_length] = '\0';
                }
            }
        }

        // Store details in global variables
        strncpy(last_method, method, sizeof(last_method));
        strncpy(last_path, path, sizeof(last_path));
        strncpy(last_content, content, sizeof(last_content));

        // Send a redirect to /confirm
        snprintf(response, sizeof(response),
                 "HTTP/1.1 302 Found\r\n"
                 "Location: /confirm\r\n"
                 "\r\n");
        send(client_socket, response, strlen(response), 0);
    } else {
        const char *response_404 = "HTTP/1.1 404 Not Found\r\n"
                                   "Content-Type: text/html\r\n"
                                   "Content-Length: 22\r\n"
                                   "\r\n"
                                   "<h1>404 Not Found</h1>";
        strncpy(response, response_404, sizeof(response));
        send(client_socket, response, strlen(response), 0);
    }

    close(client_socket);
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int opt = 1;

    // Create the server socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options to reuse address
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Configure the server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind the socket
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(server_socket, 10) < 0) {
        perror("listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server started at http://127.0.0.1:%d\n", PORT);

    // Handle client connections
    while (1) {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_socket < 0) {
            perror("accept failed");
            close(server_socket);
            exit(EXIT_FAILURE);
        }
        handle_client(client_socket);
    }

    close(server_socket);
    return 0;
}
