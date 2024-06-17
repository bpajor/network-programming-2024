#include "server.h"

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

void send_confirmation_page(int client_socket) {
    FILE *html_file = fopen("../templates/confirm.html", "r");
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