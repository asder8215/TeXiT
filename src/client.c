#include "client.h"
#include "util.h"
#include <stdio.h>
#include <gio/gio.h>
#include <gdk/gdk.h>
#include "json_object.h"

GSocketClient* client = NULL;
GSocketConnection* connection = NULL;
ShareEnableParams* client_params = NULL;
bool recreate_window = true;

static gboolean client_message_read(GIOChannel* channel, GIOCondition condition, ShareEnableParams* client_params) {
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
    
    if (recreate_window) {
        for(int i = adw_tab_view_get_n_pages(client_params->tab_view); i != 0; i = adw_tab_view_get_n_pages(client_params->tab_view)){
            Page page = get_active_page(client_params->tab_view);
            adw_tab_view_close_page(client_params->tab_view, page.page);
        }
        recreate_window = false;
    }
    
    
    json_object* jobj = json_tokener_parse(msg);
    //array_list* arr = deserialize_add_tabs(jobj);
    
    /**
    for(size_t i = 0; i < array_list_length(arr); i++){
        AddTab* tab_info = array_list_get_idx(arr, i);
        Page page = new_tab_page(client_params->tab_view, tab_info->title, NULL);
        gtk_text_buffer_set_text(GTK_TEXT_BUFFER(page.buffer), tab_info->content, -1);
    }
    **/
    
    //printf("%lu\n", array_list_length(arr));
    printf("%s\n", json_object_to_json_string_ext(jobj, JSON_C_TO_STRING_SPACED | JSON_C_TO_STRING_PRETTY));

    //adw_tab_view_close_page_finish(client_params->tab_view, );
    //adw_tab_view_close_page(client_params->tab_view, page.page);
    g_free((void*)msg);
    return TRUE;
}

// adapted mostly from drakide's stackoverflow post: https://stackoverflow.com/questions/9513327/gio-socket-server-client-example
StartStatus start_client(const char* ip_address, int port, ShareEnableParams* enable_params){
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
    
    client_params = malloc(sizeof(ShareEnableParams));

    client_params->window = enable_params->window;
    client_params->file_buttons = enable_params->file_buttons;
    client_params->label = enable_params->label;
    client_params->tabbar = enable_params->tabbar;
    client_params->toggle = enable_params->toggle;
    client_params->entries = enable_params->entries;
    client_params->toast_overlay = enable_params->toast_overlay;
    client_params->tab_view = enable_params->tab_view;

    g_io_add_watch(channel, G_IO_IN, (GIOFunc)client_message_read, client_params);

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
        free(client_params);
        client_params = NULL;
        recreate_window = true;
    }
}
