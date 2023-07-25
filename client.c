#include "client.h"
#include <stdio.h>
#include <gio/gio.h>
#include <gdk/gdk.h>

GSocketClient* client = NULL;
GSocketConnection* connection = NULL;

static gboolean client_message_read(GIOChannel* channel, GIOCondition condition, gpointer _) {
    const char* msg = NULL;
    gsize total_read = 0;
    char buf[100];
    gsize read;
    GError* error = NULL;
    
    GIOStatus status;
    while ((status = g_io_channel_read_chars(channel, buf, 99, &read, &error)) == G_IO_STATUS_NORMAL) {
        buf[read] = '\0';
        if (msg == NULL)
            msg = g_strdup_printf("%s", buf);
        else {
            const char* prev = msg;
            msg = g_strdup_printf("%s%s", msg, buf);
            g_free((void*)prev);
        }
        total_read += read;
    } 
    if (status == G_IO_STATUS_ERROR) {
        fprintf(stderr, "Error (%d) reading input stream: %s\nClosing connection... NOW\n", error->code, error->message);
        // TODO: remove callback and close connection
        g_free(error);
        return FALSE;
    }
    // TODO: remove callback if status == G_IO_STATUS_AGAIN
    printf("(Server) Received message (%lu bytes): %s\n", total_read, buf);
    
    return TRUE;
}

// adapted mostly from drakide's stackoverflow post: https://stackoverflow.com/questions/9513327/gio-socket-server-client-example
StartStatus start_client(const char* ip_address, int port){
    if (port < PORT_MIN || port > PORT_MAX) {
        return InvalidPort;
    }
    
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

    GOutputStream* ostream = g_io_stream_get_output_stream(G_IO_STREAM(connection));
    g_output_stream_write(ostream, "Test", 4, NULL, NULL);

    return Success;
}

void stop_client() {
    if (client != NULL) {
        printf("Stopping client.\n");
        GOutputStream* ostream = g_io_stream_get_output_stream(G_IO_STREAM(connection));
        g_output_stream_write(ostream, "Client left", 11, NULL, NULL);
        g_object_unref(connection);
        g_object_unref(client);
        client = NULL;
    }
}
