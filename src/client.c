#include "client.h"
#include "tab-page.h"
#include "util.h"
#include "json_types.h"
#include <stdio.h>
#include <gio/gio.h>
#include <gdk/gdk.h>
#include "json_object.h"

GSocketClient* client = NULL;
GSocketConnection* connection = NULL;

static gboolean client_message_read(GIOChannel* channel, GIOCondition condition, AdwTabView* tab_view) {
    bool closed = false;
    const char* msg = read_channel(channel, &closed);

    if (closed) {
        stop_client();
        return FALSE;
    }
    // msg == NULL but connection was not closed, happens when client reconnects.
    // Dont know why, just ignore it.
    if (msg == NULL)
        return TRUE;

    printf("(Client) Received message: %s\n", msg);
    
    json_object* tmp = NULL;
    json_object* jobj = json_tokener_parse(msg);

    // recreates the window from the server to the client
    if(json_object_object_get_ex(jobj, "add-tabs", &tmp)){
        array_list* arr = deserialize_add_tabs(tmp);
        // Append each tab received to the AdwTabView
        for(size_t i = 0; i < array_list_length(arr); i++){
            AddTab* tab_info = array_list_get_idx(arr, i);
            ClientPage page = new_client_tab(tab_view, tab_info->title);
            gtk_text_buffer_set_text(GTK_TEXT_BUFFER(page.buffer), tab_info->content, -1);
        }
    }
    else if(json_object_object_get_ex(jobj, "remove-tab", &tmp)){
        unsigned int tab_idx = json_object_get_uint64(tmp);
        AdwTabPage* page = adw_tab_view_get_nth_page(tab_view, tab_idx);
        adw_tab_view_close_page(tab_view, page);
        adw_tab_view_close_page_finish(tab_view, page, true);
    }
    else if(json_object_object_get_ex(jobj, "tab-content", &tmp)){
        TabContent* tab_content = deserialize_tab_content(tmp);
        ClientPage page = get_nth_client_tab(tab_view, tab_content->tab_idx);
        gtk_text_buffer_set_text(page.buffer, tab_content->content, -1);
    }

    json_object_put(jobj);
    g_free((void*)msg);
    return TRUE;
}

static gboolean client_close_tab_page(AdwTabView* tab_view, AdwTabPage* page, gpointer user_data){
    return GDK_EVENT_STOP;
}

// adapted mostly from drakide's stackoverflow post: https://stackoverflow.com/questions/9513327/gio-socket-server-client-example
StartStatus start_client(const char* ip_address, int port, AdwTabView* tab_view, FileButtons* file_buttons, GtkLabel* label){
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

    GSocket* socket = g_socket_connection_get_socket(connection);
    // TODO: channels needs to be freed when connection closed
    GIOChannel* channel = g_io_channel_unix_new(g_socket_get_fd(socket));

    // The client should attempt to close all tabs and hide file-buttons before connecting
    // Hide buttons
    gtk_widget_set_visible(GTK_WIDGET(file_buttons->file_new), false);
    gtk_widget_set_visible(GTK_WIDGET(file_buttons->file_open), false);
    gtk_widget_set_visible(GTK_WIDGET(file_buttons->file_save), false);
    
    // Disconnect the signal handler to the tabs in order to close them all
    // no matter if they had unsaved changes
    signal_disconnect(tab_view, close_tab_page);

    // Close all tabs the user previously had open
    while (adw_tab_view_get_n_pages(tab_view))
        adw_tab_view_close_page(tab_view, adw_tab_view_get_nth_page(tab_view, 0));
    gtk_widget_set_visible(GTK_WIDGET(tab_view), false);
    gtk_label_set_text(label, "Waiting for host to create or open a new file.");

    g_signal_connect(tab_view, "close-page", G_CALLBACK(client_close_tab_page), NULL);

    g_io_add_watch(channel, G_IO_IN, (GIOFunc)client_message_read, tab_view);

    return Success;
}

void stop_client() {
    if (client != NULL) {
        printf("Stopping client.\n");
        g_object_unref(connection);
        connection = NULL;
        g_object_unref(client);
        client = NULL;
    }
}

void client_change_tab_content(TabContent tab_content){
    const char* msg = serialize_tab_content(tab_content);
    send_message(connection, msg);
    free((void*)msg);
}
