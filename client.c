#include "client.h"
#include <stdio.h>
#include <gio/gio.h>
#include <gdk/gdk.h>

GSocketClient* client;
GSocketConnection* connection = NULL;

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
        return Other;
    }
    else {
        printf("Connected successfully!\n");
    }

    GInputStream* istream = g_io_stream_get_input_stream(G_IO_STREAM(connection));
    GOutputStream* ostream = g_io_stream_get_output_stream(G_IO_STREAM(connection));
    g_output_stream_write(ostream, "Test", 4, NULL, &error);

    if (error != NULL) {
        fprintf(stderr, "Error %d: %s\n", error->code, error->message);
        return Other;
    }

    return Success;
}

void stop_client() {
    if (client != NULL) {
        printf("Stopping client.\n");
        GError* error = NULL;
        GOutputStream* ostream = g_io_stream_get_output_stream(G_IO_STREAM(connection));
        g_output_stream_write(ostream, "Client left", 11, NULL, &error);
        g_output_stream_close(ostream, NULL, &error);
        if (error != NULL) {
            fprintf(stderr, "Error %d: %s\n", error->code, error->message);
        }
        GInputStream* istream = g_io_stream_get_input_stream(G_IO_STREAM(connection));
        g_input_stream_close(istream, NULL, &error);
        if (error != NULL) {
            fprintf(stderr, "Error %d: %s\n", error->code, error->message);
        }
        g_object_unref(connection);
        g_object_unref(client);
        client = NULL;
    }
}
