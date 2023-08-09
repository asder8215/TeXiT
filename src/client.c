#include "client.h"
#include "util.h"
#include <stdio.h>
#include <gio/gio.h>
#include <gdk/gdk.h>
#include "json_object.h"

GSocketClient* client = NULL;
GSocketConnection* connection = NULL;

static gboolean client_message_read(GIOChannel* channel, GIOCondition condition, gpointer _) {
    bool closed = false;
    const char* msg = read_channel(channel, &closed);
    if (msg == NULL) {
        printf("(Client) Could not read channel\n");
        if (!closed)
            close_connection(channel);
        closed = true;
    }

    if (closed) {
        stop_client();
        return FALSE;
    }
    // msg can't be NULL if closed is false

    printf("(Client) Received message (%lu bytes): %s\n", strlen(msg), msg);
    
    g_free((void*)msg);
    return TRUE;
}

// adapted mostly from drakide's stackoverflow post: https://stackoverflow.com/questions/9513327/gio-socket-server-client-example
StartStatus start_client(const char* ip_address, int port, AdwTabView* tab_view){
    if (port < PORT_MIN || port > PORT_MAX)
        return InvalidPort;
    if (client != NULL)
        return AlreadyStarted;
    
    const char* host_and_port = g_strdup_printf("%s:%d", ip_address, port);

    client = g_socket_client_new();
    GError* error = NULL;
    connection = g_socket_client_connect_to_host(client,
                                                 host_and_port,
                                                 0,
                                                 NULL,
                                                 &error);
    
    g_free((void*) host_and_port);

    if (error != NULL) {
        fprintf(stderr, "Error %d: %s\n", error->code, error->message);
        g_object_unref(client);
        client = NULL;
        return Other;
    }
    else {
        printf("Connected successfully!\n");
    }

    GSocket* socket = g_socket_connection_get_socket(connection);
    // TODO: channels needs to be freed when connection closed
    GIOChannel* channel = g_io_channel_unix_new(g_socket_get_fd(socket));
    g_io_add_watch(channel, G_IO_IN, (GIOFunc)client_message_read, NULL);

    return Success;
}

void stop_client() {
    if (client != NULL) {
        printf("Stopping client.\n");
        GOutputStream* ostream = g_io_stream_get_output_stream(G_IO_STREAM(connection));
        g_output_stream_write(ostream, "Client left", 11, NULL, NULL);
        g_object_unref(connection);
        connection = NULL;
        g_object_unref(client);
        client = NULL;
    }
}
