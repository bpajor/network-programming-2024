#include "server.h"

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
        send_file(client_socket, "../templates/index.html", "text/html");
    } else if (strcmp(method, "GET") == 0 && strcmp(path, "/confirm") == 0) {
        send_confirmation_page(client_socket);
    } else if (strcmp(method, "GET") == 0 && strcmp(path, "/scripts.js") == 0) {
        send_file(client_socket, "../scripts/scripts.js", "application/javascript");
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