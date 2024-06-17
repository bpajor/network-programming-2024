#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    int read_size;

    // Odczytanie żądania od klienta
    read_size = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    if (read_size < 0) {
        perror("recv failed");
        return;
    }
    buffer[read_size] = '\0';

    // Logowanie otrzymanego żądania
    printf("Received request:\n%s\n", buffer);

    // Tworzenie odpowiedzi HTTP
    char response[BUFFER_SIZE];
    memset(response, 0, BUFFER_SIZE);

    if (strncmp(buffer, "GET / ", 6) == 0) {
        FILE *html_file = fopen("index.html", "r");
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

        snprintf(response, sizeof(response),
                 "HTTP/1.1 200 OK\r\n"
                 "Content-Type: text/html\r\n"
                 "Content-Length: %ld\r\n"
                 "\r\n",
                 html_length);

        // Wysłanie nagłówków odpowiedzi
        send(client_socket, response, strlen(response), 0);

        // Wysłanie zawartości HTML
        send(client_socket, html_content, html_length, 0);

        free(html_content);
    } else if (strncmp(buffer, "POST /post ", 11) == 0) {
        const char *response_body = "POST request received";
        snprintf(response, sizeof(response),
                 "HTTP/1.1 200 OK\r\n"
                 "Content-Type: text/plain\r\n"
                 "Content-Length: %ld\r\n"
                 "\r\n%s",
                 strlen(response_body), response_body);
    } else if (strncmp(buffer, "PUT /put ", 9) == 0) {
        const char *response_body = "PUT request received";
        snprintf(response, sizeof(response),
                 "HTTP/1.1 200 OK\r\n"
                 "Content-Type: text/plain\r\n"
                 "Content-Length: %ld\r\n"
                 "\r\n%s",
                 strlen(response_body), response_body);
    } else if (strncmp(buffer, "DELETE /delete ", 15) == 0) {
        const char *response_body = "DELETE request received";
        snprintf(response, sizeof(response),
                 "HTTP/1.1 200 OK\r\n"
                 "Content-Type: text/plain\r\n"
                 "Content-Length: %ld\r\n"
                 "\r\n%s",
                 strlen(response_body), response_body);
    } else {
        const char *response_404 = "HTTP/1.1 404 Not Found\r\n"
                                   "Content-Type: text/html\r\n"
                                   "Content-Length: 22\r\n"
                                   "\r\n"
                                   "<h1>404 Not Found</h1>";
        strncpy(response, response_404, sizeof(response));
    }

    // Wysłanie odpowiedzi do klienta, jeśli nie jest to GET / (bo GET / już wysłało zawartość)
    if (strncmp(buffer, "GET / ", 6) != 0) {
        send(client_socket, response, strlen(response), 0);
    }

    close(client_socket);
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    // Tworzenie gniazda serwera
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Ustawienia adresu serwera
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bindowanie gniazda
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Nasłuchiwanie na połączenia
    if (listen(server_socket, 10) < 0) {
        perror("listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server started at http://127.0.0.1:%d\n", PORT);

    // Obsługa połączeń od klientów
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