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
#define DEFAULT_REMOTE_HOST "131.179.176.34"
#define DEFAULT_REMOTE_PORT 5001

struct server_app
{
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
    // define the server_app struct
    struct server_app app;
    // define the server_socket, client_socket for the server
    int server_socket, client_socket;
    // server_addr holds the server's address
    // client_addr holds the client's address
    struct sockaddr_in server_addr, client_addr;
    // used to keep track of how big the space is for storing the client's address when a new connection is made to your server
    socklen_t client_len;
    // return value
    int ret;

    // first parse the command-line arguments
    parse_args(argc, argv, &app);

    // create a socket for the server (TCP)
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        // if create the socket failed
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // prepare to receive incoming connections
    server_addr.sin_family = AF_INET;              // indicating the server will use IPv4 addresses.
    server_addr.sin_addr.s_addr = INADDR_ANY;      // indicates the server will listen for connections on all available interfaces
    server_addr.sin_port = htons(app.server_port); //

    // The following allows the program to immediately bind to the port in case
    // previous run exits recently
    int optval = 1;
    // Allows the server to bind to a port that was used by a previous instance of the server that may have just closed
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    // bind associates the server's socket with an address and port number
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for Incoming Connections:
    if (listen(server_socket, 10) == -1)
    { // the system can queue up to 10 incoming connections before it starts rejecting them
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", app.server_port);

    // Accept Connections in a Loop
    while (1)
    {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len); // accept a new connection
        if (client_socket == -1)                                                             // if accept failed
        {
            perror("accept failed");
            continue;
        }

        printf("Accepted connection from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port)); // print the client's address and port
        handle_request(&app, client_socket);
        close(client_socket);
    }

    close(server_socket);
    return 0;
}

// argc: count of command-line arguments
// argv[]: arguments themselves
void parse_args(int argc, char *argv[], struct server_app *app)
{
    int opt;

    app->server_port = DEFAULT_SERVER_PORT;
    app->remote_host = NULL;
    app->remote_port = DEFAULT_REMOTE_PORT;

    while ((opt = getopt(argc, argv, "b:r:p:")) != -1)
    {
        switch (opt)
        {
        case 'b': // Sets the server's listening port.
            app->server_port = atoi(optarg);
            break;
        case 'r': // Sets the remote host address for the reverse proxy functionality.
            app->remote_host = strdup(optarg);
        case 'p': // Sets the remote port for the reverse proxy to connect
            app->remote_port = atoi(optarg);
            break;
        default: /* Unrecognized parameter or "-?" */
            fprintf(stderr, "Usage: server [-b local_port] [-r remote_host] [-p remote_port]\n");
            exit(-1);
            break;
        }
    }

    // no remote host is specified, use the default remote host
    if (app->remote_host == NULL)
    {
        app->remote_host = strdup(DEFAULT_REMOTE_HOST);
    }
}

// processing incoming HTTP requests from clients
// 1. reads the request data from the "client_socket"
// 2. determine the request
// 3. determine how to reply -
//       either by serving a local file directly
//         or by proxying the request to another server (reverse proxy functionality).
void handle_request(struct server_app *app, int client_socket)
{
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    // Read the request from HTTP client
    // Note: This code is not ideal in the real world because it
    // assumes that the request header is small enough and can be read
    // once as a whole.
    // However, the current version suffices for our testing.
    bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (bytes_read <= 0)
    {
        return; // Connection closed or error
    }

    buffer[bytes_read] = '\0';
    // copy buffer to a new string
    char *request = malloc(strlen(buffer) + 1);
    strcpy(request, buffer);

    // TODO: Parse the header and extract essential fields, e.g. file name
    // Hint: if the requested path is "/" (root), default to index.html
    char file_name[] = "index.html";

    // TODO: Implement proxy and call the function under condition
    // specified in the spec
    // if (need_proxy(...)) {
    //    proxy_remote_file(app, client_socket, file_name);
    // } else {
    serve_local_file(client_socket, file_name);
    //}
}

// What does the function do:
// 1. Opening the specified file,
// 2. Reading its contents
// 3. Sending those contents back to the client over the client_socket, along with the appropriate HTTP response headers (e.g., status code, Content-Type, Content-Length).
void serve_local_file(int client_socket, const char *path)
{
    // client_socket: Used to send the content of the requested file back to the client.
    //*path: The path to the file requested by the client.
    //  -----TODO: Properly implement serving of local files-----
    //  The following code returns a dummy response for all requests
    //  but it should give you a rough idea about what a proper response looks like
    //  What you need to do
    //  (when the requested file exists):
    //  * Open the requested file
    //  * Build proper response headers (see details in the spec), and send them
    //  * Also send file content
    //  (When the requested file does not exist):
    //  * Generate a correct response

    char response[] = "HTTP/1.0 200 OK\r\n"
                      "Content-Type: text/plain; charset=UTF-8\r\n"
                      "Content-Length: 15\r\n"
                      "\r\n"
                      "Sample response";

    send(client_socket, response, strlen(response), 0);
}

void proxy_remote_file(struct server_app *app, int client_socket, const char *request)
{
    // TODO: Implement proxy request and replace the following code
    // What's needed:
    // * Connect to remote server (app->remote_server/app->remote_port)
    // * Forward the original request to the remote server
    // * Pass the response from remote server back
    // Bonus:
    // * When connection to the remote server fail, properly generate
    // HTTP 502 "Bad Gateway" response

    char response[] = "HTTP/1.0 501 Not Implemented\r\n\r\n";
    send(client_socket, response, strlen(response), 0);
}