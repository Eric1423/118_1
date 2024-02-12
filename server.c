#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <arpa/inet.h>
#include <sys/socket.h>

/**
 * Project 1 starter code
 * All parts needed to be changed/added are marked with TODO
 */

#define BUFFER_SIZE 1024
#define DEFAULT_SERVER_PORT 8081
#define DEFAULT_REMOTE_HOST "131.179.176.34" // change to "127.0.0.1 for local"
#define DEFAULT_REMOTE_PORT 5001

struct server_app {
    // Parameters of the server
    // Local port of HTTP server
    uint16_t server_port;

    // Remote host and port of remote proxy
    char *remote_host;
    uint16_t remote_port;
};

// The following function is implemented for you and doesn't need
// to be change
void parse_args(int argc, char *argv[], struct server_app *app);

// The following functions need to be updated
void handle_request(struct server_app *app, int client_socket);
void serve_local_file(int client_socket, const char *path);
void proxy_remote_file(struct server_app *app, int client_socket, const char *path);

// The main function is provided and no change is needed
int main(int argc, char *argv[])
{
    struct server_app app;
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    int ret;

    parse_args(argc, argv, &app);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(app.server_port);

    // The following allows the program to immediately bind to the port in case
    // previous run exits recently
    int optval = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 10) == -1) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", app.server_port);

    while (1) {
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket == -1) {
            perror("accept failed");
            continue;
        }
        
        printf("Accepted connection from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        handle_request(&app, client_socket);
        close(client_socket);
    }

    close(server_socket);
    return 0;
}

void parse_args(int argc, char *argv[], struct server_app *app)
{
    int opt;

    app->server_port = DEFAULT_SERVER_PORT;
    app->remote_host = NULL;
    app->remote_port = DEFAULT_REMOTE_PORT;

    while ((opt = getopt(argc, argv, "b:r:p:")) != -1) {
        switch (opt) {
        case 'b':
            app->server_port = atoi(optarg);
            break;
        case 'r':
            app->remote_host = strdup(optarg);
            break;
        case 'p':
            app->remote_port = atoi(optarg);
            break;
        default: /* Unrecognized parameter or "-?" */
            fprintf(stderr, "Usage: server [-b local_port] [-r remote_host] [-p remote_port]\n");
            exit(-1);
            break;
        }
    }

    if (app->remote_host == NULL) {
        app->remote_host = strdup(DEFAULT_REMOTE_HOST);
    }
}

void handle_request(struct server_app *app, int client_socket) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    // Read the request from HTTP client
    // Note: This code is not ideal in the real world because it
    // assumes that the request header is small enough and can be read
    // once as a whole.
    // However, the current version suffices for our testing.
    bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (bytes_read <= 0) {
        return;  // Connection closed or error
    }

    buffer[bytes_read] = '\0';
    // copy buffer to a new string
    char *request = malloc(strlen(buffer) + 1);
    strcpy(request, buffer);

    // TODO: Parse the header and extract essential fields, e.g. file name
    // Hint: if the requested path is "/" (root), default to index.html
    char file_name[BUFFER_SIZE];
    // char *file_path = strtok(NULL, " ");
    char *method = strtok(request, " ");
    if (strcmp(method, "GET") == 0) {
        char *file_path = strtok(NULL, " ");
        if (strcmp(file_path, "/") == 0) {
            strcpy(file_name, "index.html"); 
        } else {
            // if start with leading '/'
            if (file_path[0] == '/') {
                file_path++;
            }
            snprintf(file_name, sizeof(file_name), "%s", file_path);
        }
    }
    

    // TODO: Implement proxy and call the function under condition
    // specified in the spec
    // if (need_proxy(...)) {
    //    proxy_remote_file(app, client_socket, file_name);
    // } else {
    // serve_local_file(client_socket, file_name);
    //}
    if (strstr(file_name, ".ts")) {
        proxy_remote_file(app, client_socket, file_name);
    } else {
        serve_local_file(client_socket, file_name);
    }
    // char *method, *path, *protocol;
    // method = strtok(request, " ");
    // path = strtok(NULL, " ");
    // protocol = strtok(NULL, "\r\n"); // Protocol is not used but extracted for completeness

    // if (method && path) {
    //     // Default to serving index.html for root "/"
    //     if (strcmp(path, "/") == 0) {
    //         path = "/index.html";
    //     }

    //     if (strstr(path, ".ts")) {
    //         // Proxy request for .ts files
    //         proxy_remote_file(app, client_socket, path + 1); // +1 to skip the leading '/'
    //     } else {
    //         // Serve local files directly
    //         serve_local_file(client_socket, path + 1); // +1 to skip the leading '/'
    //     }
    // }


    // Memory cleanup
    free(request);
}

void serve_local_file(int client_socket, const char *path) {
    // TODO: Properly implement serving of local files
    // The following code returns a dummy response for all requests
    // but it should give you a rough idea about what a proper response looks like
    // What you need to do 
    // (when the requested file exists):
    // * Open the requested file
    // * Build proper response headers (see details in the spec), and send them
    // * Also send file content
    // (When the requested file does not exist):
    // * Generate a correct response

    // Open the requested file
    FILE *file = fopen(path, "rb");
    // If the file exists
    if (file) {
        // Find the content length
        fseek(file, 0, SEEK_END);
        long c_length = ftell(file);
        rewind(file);

        // Default content type to binary
        const char *c_type = "application/octet-stream"; 
        // Determine type extension
        if (strstr(path, ".html")) {
            c_type = "text/html; charset=UTF-8";
        } else if (strstr(path, ".txt")) {
            c_type = "text/plain; charset=UTF-8";
        } else if (strstr(path, ".jpg")) {
            c_type = "image/jpeg";
        } else if (strstr(path, ".m3u8")) {
            c_type = "application/vnd.apple.mpegurl";
        } else if (strstr(path, ".ts")) {
            c_type = "video/MP2T";
        }

        // HTTP header
        char header[BUFFER_SIZE];
        sprintf(header, 
            "HTTP/1.0 200 OK\r\n"
            "Content-Type: %s\r\n"
            "Content-Length: %ld\r\n"
            "\r\n", 
            c_type, c_length);
        send(client_socket, header, strlen(header), 0);

        // Send content
        char *buffer = malloc(BUFFER_SIZE);
        if (buffer) {
            size_t bytes_read;
            while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
                send(client_socket, buffer, bytes_read, 0);
            }
            free(buffer);
        }
        fclose(file);
    } else {
        // File not found
        char response[] = 
            "HTTP/1.0 404 Not Found\r\n"
            "Content-Type: text/html\r\n"
            "\r\n"
            "<html><body><h1>404 Not Found</h1></body></html>";
        send(client_socket, response, strlen(response), 0);
    }

    // char response[] = "HTTP/1.0 200 OK\r\n"
    //                   "Content-Type: text/plain; charset=UTF-8\r\n"
    //                   "Content-Length: 15\r\n"
    //                   "\r\n"
    //                   "Sample response";

    // send(client_socket, response, strlen(response), 0);
}

void proxy_remote_file(struct server_app *app, int client_socket, const char *request) {
    // TODO: Implement proxy request and replace the following code
    // What's needed:
    // * Connect to remote server (app->remote_server/app->remote_port)
    // * Forward the original request to the remote server
    // * Pass the response from remote server back
    // Bonus:
    // * When connection to the remote server fail, properly generate
    // HTTP 502 "Bad Gateway" response

    // char response[] = "HTTP/1.0 501 Not Implemented\r\n\r\n";
    // send(client_socket, response, strlen(response), 0);

   // Connect to remote server
    int remote_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (remote_socket < 0) {
        perror("Creating socket failed");
        return;
    }
    printf("Socket created successfully\n");

    struct sockaddr_in remote_addr;
    memset(&remote_addr, 0, sizeof(remote_addr));
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_addr.s_addr = inet_addr(app->remote_host);
    remote_addr.sin_port = htons(app->remote_port);

    if (inet_pton(AF_INET, app->remote_host, &remote_addr.sin_addr) <= 0) {
        perror("Invalid remote IP address");
        close(remote_socket);
        return;
    }
    printf("Remote address set successfully\n");

    // Establish connection
    if (connect(remote_socket, (struct sockaddr *)&remote_addr, sizeof(remote_addr)) < 0) {
        perror("Connection fail");
        close(remote_socket);
        // Print additional diagnostic information
        fprintf(stderr, "Failed to connect to remote server %s:%d\n", app->remote_host, app->remote_port);
        return;
    }
    printf("Connected to remote server successfully\n");

    // Forward original request to the remote server
    char proxy_request[BUFFER_SIZE];
    snprintf(proxy_request, sizeof(proxy_request), 
        "GET /%s HTTP/1.1\r\n"
        "Host: %s\r\n"
        "Connection: close\r\n"
        "\r\n", 
        request, app->remote_host);
    printf("Proxy request:\n%s\n", proxy_request);

    send(remote_socket, proxy_request, strlen(proxy_request), 0);
    printf("Request sent to remote server successfully\n");

    // Pass the response from remote server back
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    while ((bytes_read = recv(remote_socket, buffer, BUFFER_SIZE, 0)) > 0) {
        send(client_socket, buffer, bytes_read, 0);
    }
    printf("Response received from remote server and sent to client\n");

    // Close sockets
    close(remote_socket);
    printf("Remote socket closed\n");

}