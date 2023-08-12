#include "server.h"
#include "buffer.h"
#include "tab-page.h"
#include "gui.h"
#include <stdio.h>
#include <gio/gio.h>
#include <gdk/gdk.h>
#include <string.h>

GSocketService* server = NULL;
// TODO: store channels inteaddd of connections???
GSocketConnection* server_connections[MAX_CONNECTIONS];
unsigned int server_connections_count = 0;
AdwTabView* tab_view = NULL;
GtkToggleButton* server_toggle = NULL;

static void remove_connection(GSocketConnection* connection) {
    unsigned int i;
    bool found = false;
    // Look for target connection in the array
    for (i = 0; i < MAX_CONNECTIONS; i++) {
        if (server_connections[i] == connection) {
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

    g_object_unref(connection);
}

static gboolean server_message_read(GIOChannel* channel, GIOCondition condition, GSocketConnection* connection) {
    bool closed = false;
    const char* msg = read_channel(channel, &closed);
    
    if (closed) {
        remove_connection(connection);
        return FALSE;
    }
    // Server getting NULL msg and connection not closed has never happened.
    // But just in case, just ignore it.
    if (msg == NULL)
        return TRUE;

    printf("(Server) Received message: %s\n", msg);
    
    json_object* tmp;
    json_object* jobj = json_tokener_parse(msg);

    // Interpret the message
    if(json_object_object_get_ex(jobj, "tab-content", &tmp)){
        TabContent* tab_content = deserialize_tab_content(tmp);
        Page page = get_nth_page(tab_view, tab_content->tab_idx);
        // Set the content of the TextBuffer
        gtk_text_buffer_set_text(GTK_TEXT_BUFFER(page.buffer), tab_content->content, -1);
        // Send the change to all other clients
        for (unsigned int i = 0; i < server_connections_count; i++) {
            GSocketConnection* c = server_connections[i];
            if (c != connection)
                send_message(c, msg);
        }

        tab_content_free(*tab_content);
        free(tab_content);
    }
    
    json_object_put(jobj);
    g_free((void*)msg);
    return TRUE;
}

/// Handler for when the server gets a new connection request.
static gboolean server_new_incoming(GSocketService* server, GSocketConnection* connection, GObject* _, gpointer unused) {
    if (server_connections_count == MAX_CONNECTIONS) {
        fprintf(stderr, "Attempted new connection, but Reached maximum number of connections (%d)\n", MAX_CONNECTIONS);
        return GDK_EVENT_PROPAGATE;
    }
    server_connections[server_connections_count] = connection;
    server_connections_count++;
    g_object_ref(connection);

    printf("(Server) New connection (%p)\n", connection);

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
StartStatus start_server(int port, AdwTabView* p_tab_view, GtkToggleButton* p_toggle) {
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
    tab_view = p_tab_view;
    server_toggle = p_toggle;

    // Set the state of the toggle button to HOSTING
    gtk_button_set_label(GTK_BUTTON(server_toggle), TOGGLE_LABEL_HOSTING);
    gtk_toggle_button_set_active(server_toggle, true);

    g_signal_connect(server, "incoming", G_CALLBACK(server_new_incoming), NULL);
    g_socket_service_start(server);
    
    return Success;
}

void stop_server() {
    if (server != NULL) {
        printf("Stopping server.\n");

        // Set the state of the toggle button to OFF
        gtk_button_set_label(GTK_BUTTON(server_toggle), TOGGLE_LABEL_OFF);
        gtk_toggle_button_set_active(server_toggle, false);

        g_socket_service_stop(server);
        g_socket_listener_close(G_SOCKET_LISTENER(server));
        g_object_unref(server);
        for (unsigned int i = 0; i < server_connections_count; i++) {
            g_object_unref(server_connections[i]);
            server_connections[i] = NULL;
        }
        server_connections_count = 0;
        server = NULL;
        // These should not be freed, that's done when the window closes
        tab_view = NULL;
        server_toggle = NULL;
    }
}

void server_new_tab() {
    // Function can be called before server has started.
    if (server == NULL)
        return;

    Page page;
    AddTab* new_tab = malloc(sizeof(AddTab));
    // The new tab is always the last one
    new_tab->tab_idx = adw_tab_view_get_n_pages(tab_view) - 1;

    page = get_nth_page(tab_view, new_tab->tab_idx);
    new_tab->title = strdup(adw_tab_page_get_title(page.page));
    new_tab->content = editor_buffer_get_content(page.buffer);

    // Send the message to all connections
    const char* msg = serialize_add_tabs(new_tab, 1);
    for (unsigned int i = 0; i < server_connections_count; i++)
        send_message(server_connections[i], msg);
    free((void*)msg);
}

void server_remove_tab(AdwTabPage* target_page){
    if (server == NULL)
        return;
    
    unsigned int tab_idx = adw_tab_view_get_page_position(tab_view, target_page);
    const char* msg = serialize_remove_tab(tab_idx);

    for(unsigned int i = 0; i < server_connections_count; i++)
        send_message(server_connections[i], msg);
    free((void*)msg);
}

void server_change_tab_content(TabContent tab_content){
    if (server == NULL)
        return;
    
    const char* msg = serialize_tab_content(tab_content);

    for(unsigned int i = 0; i < server_connections_count; i++)
        send_message(server_connections[i], msg);
    free((void*)msg);
}
