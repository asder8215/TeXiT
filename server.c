#include "server.h"
#include "gui.h"
#include <stdio.h>
#include <gio/gio.h>
#include <gdk/gdk.h>
#include <string.h>

GSocketService* server = NULL;
GSocketConnection* server_connections[MAX_CONNECTIONS];
unsigned int server_connections_count = 0;


static void server_message_read(GIOChannel* channel, GIOCondition condition, GSocketConnection* connection) {
    char* msg = NULL;
    gsize msg_len = 0;
    GError* error = NULL;
    printf("channel: %p, condition: %d, connection: %p\n", channel, condition, connection);

    if (g_io_channel_read_to_end(channel, &msg, &msg_len, &error) == G_IO_STATUS_ERROR) {
        fprintf(stderr, "Error (%d) reading input stream: %s\nClosing connection... NOW\n", error->code, error->message);
        // TODO: close connection
        g_free(error);
        return;
    }

    printf("Received message (%lu bytes): %s\n", msg_len, msg);
    g_free(msg);
}

/// Handler for when the server gets a new connection request.
static gboolean server_new_incoming(GSocketService* server, GSocketConnection* connection, GObject* _, gpointer unused_data) {
    printf("New connection\n");

    if (server_connections_count == MAX_CONNECTIONS) {
        fprintf(stderr, "Reached maximum number of connections (%d)\n", MAX_CONNECTIONS);
        return GDK_EVENT_PROPAGATE;
    } else {
        server_connections[server_connections_count] = connection;
        server_connections_count++;
        g_object_ref(connection);
    }

    GSocket* socket = g_socket_connection_get_socket(connection);
    // TODO: channels needs to be freed when connection closed
    GIOChannel* channel = g_io_channel_unix_new(g_socket_get_fd(socket));
    g_io_add_watch(channel, G_IO_IN, (GIOFunc)server_message_read, connection);

    GOutputStream* ostream = g_io_stream_get_output_stream(G_IO_STREAM(connection));
    const char* msg = "Welcome new user 😎";
    g_output_stream_write_all(ostream, msg, strlen(msg), NULL, NULL, NULL);
    g_output_stream_flush(ostream, NULL, NULL);
    
    return GDK_EVENT_PROPAGATE;
}

// adapted mostly from drakide's stackoverflow post: https://stackoverflow.com/questions/9513327/gio-socket-server-client-example
StartStatus start_server(int port) {
    if (port < PORT_MIN || port > PORT_MAX) {
        return InvalidPort;
    }
    if (server != NULL)
        return AlreadyStarted;

    server = g_socket_service_new();
    GError* error = NULL;
    if (!g_socket_listener_add_inet_port(G_SOCKET_LISTENER(server), port, NULL, &error)) {
        if (error == NULL) {
            fprintf(stderr, "Whar??? g_socket_listener_add_inet_port returned 'false', but error is NULL.\n");
        } else {
            fprintf(stderr, "Error %d: %s\n", error->code, error->message);
            g_object_unref(error);
        }
        return Other;
    };

    g_signal_connect(server, "incoming", G_CALLBACK(server_new_incoming), NULL);
    g_socket_service_start(server);

    return Success;
}

void stop_server() {
    if (server != NULL) {
        printf("Stopping server.\n");
        g_socket_service_stop(server);
        g_object_unref(server);
        for (unsigned int i = 0; i < server_connections_count; i++) {
            g_object_unref(server_connections[i]);
            server_connections[i] = NULL;
        }
        server_connections_count = 0;
        server = NULL;
    }
}
