#include "server.h"
#include "gui.h"
#include "util.h"
#include <stdio.h>
#include <gio/gio.h>
#include <gdk/gdk.h>
#include <string.h>

GSocketService* server = NULL;
// TODO: store channels inteaddd of connections???
GSocketConnection* server_connections[MAX_CONNECTIONS];
unsigned int server_connections_count = 0;

static void remove_connection(GSocketConnection* target) {
    unsigned int i;
    bool found = false;
    // Look for target in the array
    for (i = 0; i < MAX_CONNECTIONS; i++) {
        if (server_connections[i] == target) {
            found = true;
            break;
        }
    }
    if (!found) {
        fprintf(stderr, "Error: Tried removing a conneciton that is not in the array\n");
        return;
    }
    // And move each connection after 1 index back
    for (i++; i < MAX_CONNECTIONS; i++) {
        server_connections[i - 1] = server_connections[i];
    }

    server_connections[MAX_CONNECTIONS - 1] = NULL;
    server_connections_count--;

    g_object_unref(target);
}

static gboolean server_message_read(GIOChannel* channel, GIOCondition condition, GSocketConnection* connection) {
    bool closed = false;
    const char* msg = read_channel(channel, &closed);
    if (msg == NULL) {
        printf("(Server) Could not read channel\n");
        if (!closed)
            close_connection(channel);
        closed = true;
    }
    
    if (closed) {
        remove_connection(connection);
        return FALSE;
    }
    // msg can't be NULL if closed is false

    printf("(Server) Received message (%lu bytes): %s\n", strlen(msg), msg);

    g_free((void*)msg);
    return TRUE;
}

/// Handler for when the server gets a new connection request.
static gboolean server_new_incoming(GSocketService* server, GSocketConnection* connection, GObject* _, AdwTabView* tab_view) {
    if (server_connections_count == MAX_CONNECTIONS) {
        fprintf(stderr, "Attempted new connection, but Reached maximum number of connections (%d)\n", MAX_CONNECTIONS);
        return GDK_EVENT_PROPAGATE;
    }
    server_connections[server_connections_count] = connection;
    server_connections_count++;
    g_object_ref(connection);

    printf("New connection\n");

    GSocket* socket = g_socket_connection_get_socket(connection);
    // TODO: channels needs to be freed when connection closed
    GIOChannel* channel = g_io_channel_unix_new(g_socket_get_fd(socket));
    g_io_add_watch(channel, G_IO_IN, (GIOFunc)server_message_read, connection);

    // Send currently opened tabs
    const char* msg = serialize_add_tabs_from_view(tab_view);
    send_message(connection, msg);
    free((void*)msg);
    
    return GDK_EVENT_PROPAGATE;
}

// adapted mostly from drakide's stackoverflow post: https://stackoverflow.com/questions/9513327/gio-socket-server-client-example
StartStatus start_server(int port, AdwTabView* tab_view) {
    if (port < PORT_MIN || port > PORT_MAX)
        return InvalidPort;
    if (server != NULL)
        return AlreadyStarted;

    server = g_socket_service_new();
    GError* error = NULL;
    if (!g_socket_listener_add_inet_port(G_SOCKET_LISTENER(server), port, NULL, &error)) {
        if (error == NULL) {
            fprintf(stderr, "Whar??? g_socket_listener_add_inet_port returned 'false', but error is NULL.\n");
        } else {
            fprintf(stderr, "Error %d: %s\n", error->code, error->message);
            g_free(error);
        }
        return Other;
    };

    g_signal_connect(server, "incoming", G_CALLBACK(server_new_incoming), tab_view);
    g_socket_service_start(server);
    
    return Success;
}

void stop_server() {
    if (server != NULL) {
        printf("Stopping server.\n");
        g_socket_service_stop(server);
        g_socket_listener_close(G_SOCKET_LISTENER(server));
        g_object_unref(server);
        for (unsigned int i = 0; i < server_connections_count; i++) {
            g_object_unref(server_connections[i]);
            server_connections[i] = NULL;
        }
        server_connections_count = 0;
        server = NULL;
    }
}
