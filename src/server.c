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

typedef struct {
    GSocketConnection* connection;
    AdwTabView* tab_view;
} TabViewConnection;

static void remove_connection(TabViewConnection* tab_view_with_connection) {
    unsigned int i;
    bool found = false;
    // Look for target connection in the array
    for (i = 0; i < MAX_CONNECTIONS; i++) {
        if (server_connections[i] == tab_view_with_connection->connection) {
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

    g_object_unref(tab_view_with_connection->connection);
    free(tab_view_with_connection);
}

static gboolean server_message_read(GIOChannel* channel, GIOCondition condition, TabViewConnection* tab_view_with_connection) {
    bool closed = false;
    const char* msg = read_channel(channel, &closed);
    
    if (closed) {
        remove_connection(tab_view_with_connection);
        return FALSE;
    }
    // Server getting NULL msg and connection not closed has never happened.
    // But just in case, just ignore it.
    if (msg == NULL)
        return TRUE;

    printf("(Server) Received message: %s\n", msg);
    
    json_object* tmp;
    json_object* jobj = json_tokener_parse(msg);

    if(json_object_object_get_ex(jobj, "tab-content", &tmp)){
        //unsigned int tab_idx = json_
        //printf("%s\n", json_object_to_json_string(tmp));
        json_object *tab_idx_json, *content_json;
        json_object_object_get_ex(tmp, "tab-idx", &tab_idx_json);
        json_object_object_get_ex(tmp, "content", &content_json);
        unsigned int tab_idx = json_object_get_uint64(tab_idx_json);
        const char* content = json_object_get_string(content_json);
        AdwTabPage* page = adw_tab_view_get_nth_page(tab_view_with_connection->tab_view, tab_idx);
        EditorBuffer* buffer = page_get_buffer(page);
        gtk_text_buffer_set_text(GTK_TEXT_BUFFER(buffer), content, -1);
    }
    
    json_object_put(jobj);
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

    printf("(Server) New connection (%p)\n", connection);

    GSocket* socket = g_socket_connection_get_socket(connection);
    // TODO: channels needs to be freed when connection closed
    GIOChannel* channel = g_io_channel_unix_new(g_socket_get_fd(socket));
    
    TabViewConnection* tab_view_with_connection = malloc(sizeof(TabViewConnection));
    tab_view_with_connection->connection = connection;
    tab_view_with_connection->tab_view = tab_view;
    g_io_add_watch(channel, G_IO_IN, (GIOFunc)server_message_read, tab_view_with_connection);

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

void server_new_tab(AdwTabView* tab_view) {
    // Function can be called before server has started.
    if (server == NULL)
        return;

    Page page;
    AddTab* new_tab = malloc(sizeof(AddTab));
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

void server_remove_tab(AdwTabView* tab_view, AdwTabPage* target_page){
    if (server == NULL)
        return;
    
    unsigned int tab_idx = adw_tab_view_get_page_position(tab_view, target_page);
    const char* msg = serialize_remove_tab(tab_idx);

    for(unsigned int i = 0; i < server_connections_count; i++)
        send_message(server_connections[i], msg);
    free((void*)msg);
}

void server_change_tab_content(const char* content, unsigned int tab_idx){
    if (server == NULL)
        return;
    
    const char* msg = serialize_tab_content(content, tab_idx);

    for(unsigned int i = 0; i < server_connections_count; i++)
        send_message(server_connections[i], msg);

    free((void*)msg);
}
