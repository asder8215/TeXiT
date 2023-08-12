#include "util.h"
#include "arraylist.h"
#include "buffer.h"
#include "json_tokener.h"
#include "tab-page.h"
#include "json.h"
#include "json_object.h"
#include "json_types.h"
#include <stdio.h>

/// JSON value keys used for creating and accessing json objects.
static const char* ADD_TABS_KEY = "add-tabs";
static const char* RM_TAB_KEY = "remove-tab";
static const char* RENAME_TABS_KEY = "rename-tabs";
static const char* DEL_CONTENT_KEY = "delete-content";
static const char* REPLACE_CONTENT_KEY = "replace-content";
static const char* INSERT_CONTENT_KEY = "insert-content";
static const char* TAB_CONTENT_KEY = "tab-content";

const char* read_channel(GIOChannel* channel, bool* closed) {
    const char* msg = NULL;
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

void send_message(GSocketConnection* connection, const char* msg) {
    GOutputStream* ostream = g_io_stream_get_output_stream(G_IO_STREAM(connection));
    // strlen(msg) + 1 to include the '\0'
    g_output_stream_write(ostream, msg, strlen(msg), NULL, NULL);
}

void close_connection(GIOChannel* channel) {
    GError* error = NULL;

    g_io_channel_shutdown(channel, true, NULL);
    g_io_channel_unref(channel);
    // Connnection doesnt have to be clsoed once channel is closed
}

void signal_disconnect(gpointer instance, gpointer callback) {
    gulong handler_id = g_signal_handler_find(instance, G_SIGNAL_MATCH_FUNC, -1, 0, NULL, callback, NULL);
    g_signal_handler_disconnect(instance, handler_id);
}


inline void add_tab_free(AddTab self) {
    free((void*)self.title);
    free((void*)self.content);
}
/// Necessary to be able to use AddTab in array_list.
void add_tab_free_ptr(void* ptr) {
    add_tab_free(*(AddTab*)ptr);
    free(ptr);
}
inline void rename_tab_free(RenameTab self) {
    free((void*)self.title);
}
inline void replace_content_free(ReplaceContent self) {
    free((void*)self.content);
}
inline void insert_content_free(InsertContent self) {
    free((void*)self.content);
}
inline void tab_content_free(TabContent self) {
    free((void*)self.content);
}


// /// (FOR USE IN DESERIALIZATION FUNCTIONS ONLY).
// /// Assigns the value to **target_field** if the value found in **src_obj** is of a certain type.
// /// Makes the function it is called in return NULL and frees values if it fails.
// #define json_assign_or_fail(field_type, field_container, target_field, obj_type, key, src_obj) \
//     tmp_obj = json_object_object_get(src_obj, key);                                            \
//     if (tmp_obj == NULL || json_object_get_type(tmp_obj) != json_type_##obj_type) {            \
//         free(field_container);                                                                 \
//         array_list_free(rtrn);                                                                 \
//         return NULL;                                                                           \
//     }                                                                                          \
//     field_container->target_field = json_object_get_##field_type(tmp_obj);

/// Assign the value of src_obj["key"] to tmp_obj if it is of a certain type.
/// Makes the function it is called in return NULL and frees the malloced return value if it fails.
#define json_get_or_fail(type, key, src_obj)                                    \
    tmp_obj = json_object_object_get(src_obj, key);                             \
    if (tmp_obj == NULL || json_object_get_type(tmp_obj) != json_type_##type) { \
        free(rtrn);                                                             \
        return NULL;                                                            \
    }
/// Like `json_get_or_fail`, but the return value is a list and also frees a malloced value.
#define json_get_or_fail_list(type, key, src_obj, free_val)                     \
    tmp_obj = json_object_object_get(src_obj, key);                             \
    if (tmp_obj == NULL || json_object_get_type(tmp_obj) != json_type_##type) { \
        free(free_val);                                                         \
        array_list_free(rtrn);                                                  \
        return NULL;                                                            \
    }

array_list* deserialize_add_tabs(json_object* list) {
    array_list* rtrn = NULL;

    // Get the array assigned to "add-tabs" key.
    if (list == NULL || json_object_get_type(list) != json_type_array)
        return NULL;

    // Allocate the return array. The array can still become NULL if later deserialization fails
    rtrn = array_list_new2(add_tab_free_ptr, json_object_array_length(list));

    for (size_t i = 0; i < json_object_array_length(list); i++) {
        json_object* tmp_obj;
        json_object* add_tab_obj = json_object_array_get_idx(list, i);
        AddTab* add_tab = malloc(sizeof(AddTab));
        
        json_get_or_fail_list(int, "tab-idx", add_tab_obj, add_tab);
        add_tab->tab_idx = json_object_get_uint64(tmp_obj);
        
        json_get_or_fail_list(string, "title", add_tab_obj, add_tab);
        add_tab->title = strdup(json_object_get_string(tmp_obj));

        json_get_or_fail_list(string, "content", add_tab_obj, add_tab);
        add_tab->content = strdup(json_object_get_string(tmp_obj));

        array_list_add(rtrn, add_tab);
    }

    return rtrn;
}

TabContent* deserialize_tab_content(json_object* obj) {
    if (obj == NULL || json_object_get_type(obj) != json_type_object)
        return NULL;

    json_object* tmp_obj;
    TabContent* rtrn = malloc(sizeof(TabContent));

    json_get_or_fail(int, "tab-idx", obj);
    rtrn->tab_idx = json_object_get_uint64(tmp_obj);
    json_get_or_fail(string, "content", obj);
    rtrn->content = strdup(json_object_get_string(tmp_obj));

    return rtrn;
}


const char* serialize_add_tabs_from_view(AdwTabView* tab_view) {
    size_t len = adw_tab_view_get_n_pages(tab_view);
    AddTab* add_tabs = malloc(sizeof(AddTab) * len);

    for (size_t i = 0; i < len; i++) {
        Page page = get_nth_page(tab_view, i);
        AddTab add_tab;
        add_tab.tab_idx = i;
        add_tab.title = strdup(adw_tab_page_get_title(page.page));
        add_tab.content = editor_buffer_get_content(page.buffer);

        add_tabs[i] = add_tab;
    }

    return serialize_add_tabs(add_tabs, len);
}
const char* serialize_add_tabs(AddTab* add_tabs, size_t len) {
    json_object* obj = json_object_new_object();
    json_object* arr = json_object_new_array_ext(len);
    const char* rtrn = NULL;

    // Fill the array_list
    for (size_t i = 0; i < len; i++) {
        AddTab add_tab = add_tabs[i];
        json_object* add_tab_obj = json_object_new_object();
        json_object_object_add(add_tab_obj, "tab-idx", json_object_new_uint64(add_tab.tab_idx));
        json_object_object_add(add_tab_obj, "title", json_object_new_string(add_tab.title));
        json_object_object_add(add_tab_obj, "content", json_object_new_string(add_tab.content));

        json_object_array_add(arr, add_tab_obj);
    }
    json_object_object_add(obj, ADD_TABS_KEY, arr);
    rtrn = strdup(json_object_to_json_string(obj));

    json_object_put(obj);
    for (size_t i = 0; i < len; i++)
        add_tab_free(add_tabs[i]);
    free(add_tabs);
    return rtrn;
}

const char* serialize_remove_tab(TabIdx tab_idx){
    json_object* obj = json_object_new_object();
    const char* rtrn = NULL;
    
    json_object_object_add(obj, RM_TAB_KEY, json_object_new_uint64(tab_idx));
    rtrn = strdup(json_object_to_json_string(obj));

    json_object_put(obj);
    return rtrn;
}

// TODO: This function may be deprecated if a better solution is found to changing
// content on both server and client side.
const char* serialize_tab_content(TabContent tab_content){
    json_object* obj = json_object_new_object();
    json_object* inner_json = json_object_new_object();
    const char* rtrn = NULL;

    json_object_object_add(inner_json, "content", json_object_new_string(tab_content.content));
    json_object_object_add(inner_json, "tab-idx", json_object_new_uint64(tab_content.tab_idx));

    json_object_object_add(obj, TAB_CONTENT_KEY, inner_json);
    rtrn = strdup(json_object_to_json_string(obj));

    //json_object_put(inner_json);
    json_object_put(obj);
    tab_content_free(tab_content);

    return rtrn;
}
