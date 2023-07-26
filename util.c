#include "util.h"
#include <stdio.h>

const char* read_channel(GIOChannel* channel, bool* closed) {
    const char* msg = NULL;
    gsize total_read = 0;
    char buf[100];
    gsize read;
    GError* error = NULL;
    
    GIOStatus status;
    while ((status = g_io_channel_read_chars(channel, buf, 99, &read, &error)) == G_IO_STATUS_NORMAL) {
        buf[read] = '\0';
        if (read > 0) {
            if (msg == NULL)
                msg = g_strdup_printf("%s", buf);
            else {
                const char* prev = msg;
                msg = g_strdup_printf("%s%s", msg, buf);
                g_free((void*)prev);
            }
            total_read += read;
        }
    }

    switch (status) {
        case G_IO_STATUS_ERROR:
            fprintf(stderr, "Error (%d) reading channel: %s\n", error->code, error->message);
            g_free(error);
            // proceed to close channel in EOF
        case G_IO_STATUS_EOF:
            close_connection(channel);
            *closed = true;
            printf("(read_channnel) Closed channel\n");
            break;
        case G_IO_STATUS_AGAIN:
            // waiting for next message...
            break;
        case G_IO_STATUS_NORMAL:
            // will not happen
            break;
    }
    
    return msg;
}

void close_connection(GIOChannel* channel) {
    GError* error = NULL;

    g_io_channel_shutdown(channel, true, NULL);
    g_io_channel_unref(channel);
    // Connnection doesnt have to be clsoed once channel is closed
}
