#include "server.h"
#include <stdio.h>
#include <gio/gio.h>
#include <gdk/gdk.h>

GSocketService* server = NULL;

/// Handler for when the server gets a new connection request.
static gboolean server_new_incoming(GSocketService* server, GSocketConnection* connection, GObject* _, gpointer unused_data) {
    printf("New connection\n");
    // TODO:
    return GDK_EVENT_PROPAGATE;
}



StartServerStatus start_server(int port) {
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
        server = NULL;
    }
}
