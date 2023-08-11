#ifndef __UTIL_H__
#define __UTIL_H__

#include "json.h"
#include "gui.h"
#include <gdk/gdk.h>
#include <adwaita.h>
#include <stdbool.h>

#define PORT_MIN 1024
#define PORT_MAX 49151
#define DEFAULT_PORT 33378

typedef enum  {
    Success,
    AlreadyStarted,
    InvalidPort,
    Other,
} StartStatus;

/// Can return NULL.
/// Function takes ownership of **channel** (could be freed), so don't use after this call.
/// **connection** is closed with the **channel**.
/// **closed** is set to `true` if channel and conneciton were closed.
/// Channel can be closed if reached EOF or Error.
/// 
/// Return value must be `g_free`d.
const char* read_channel(GIOChannel* channel, bool* closed);
/// Sends a message through the **connection**. Does NOT sends the '\0' character
/// Caller owns **connection** and **msg**.
void send_message(GSocketConnection* connection, const char* msg);
/// When the channel is closed, so is the connection (at least thats what it seems like).
void close_connection(GIOChannel* channel);


// JSON serialization/deserialization

/// Has ownership of title and content.
typedef struct {
    unsigned int tab_idx;
    const char* title;
    const char* content;
} AddTab;
// AddTab add_tab_new(unsigned int tab_idx, const char* title, const char* content);
void add_tab_free(AddTab self);


/// Has ownership of title.
/// Should be malloced and freed.
typedef struct {
    unsigned int tab_idx;
    const char* title;
} RenameTab;
// RenameTab* rename_tab_new(unsigned int tab_idx, const char* title);
void rename_tab_free(RenameTab self);

// Does not have to be malleced or freed
typedef struct {
    unsigned int tab_idx;
    size_t start_byte;
    size_t byte_length; 
} DeleteContent;

/// Is basically a combination of DeleteContent and InsertContent;
/// Has ownership of content.
/// Should be malloced and freed.
typedef struct {
    unsigned int tab_idx;
    size_t start_byte;
    size_t byte_length;
    const char* content;
} ReplaceContent;
// ReplaceContent* replace_content_new(unsigned int tab_idx, size_t start_byte, size_t byte_length, const char* content);
void replace_content_free(ReplaceContent self);

/// Has ownership of content.
/// Should be malloced and freed.
typedef struct {
    unsigned int tab_idx;
    size_t start_byte;
    const char* content;
} InsertContent;
// InsertContent* insert_content_new(unsigned int tab_idx, size_t start_byte, const char* content);
void insert_content_free(InsertContent self);

typedef enum {
    MSG_T_ADD_TABS,
    MSG_T_RM_TABS,
    MSG_T_RENAME_TABS,
    MSG_T_DEL_CONTENT,
    MSG_T_REPLACE_CONTENT,
    MSG_T_INSERT_CONTENT
} MessageType;

/// Deserialize a json list into a **list of  AddTab** structs.
/// **list** is a borrowed reference.
/// Returns NULL if deserialization fails.
array_list* deserialize_add_tabs(json_object* list);
/// Deserialize a json list into a **list of RemoveTab** structs.
/// **json** is a borrowed reference.
/// array_list.array NULL if deserialization fails.
array_list deserialize_rename_tabs(const char* json);
/// Returns NULL if deserialization fails.
/// **json** is a borrowed reference.
/// Caller takes ownership of return value and must free it.
DeleteContent* deserialize_delete_content(const char* json);
/// Returns NULL if deserialization fails.
/// **json** is a borrowed reference.
/// Caller takes ownership of return value and must free it.
ReplaceContent* deserialize_replace_content(const char* json);
/// Returns NULL if deserialization fails.
/// **json** is a borrowed reference.
/// Caller takes ownership of return value and must free it.
InsertContent* deserialize_insert_content(const char* json);

/// TODO: maybe use array_list for the following?

/// Create a *list of AddTab* structs from all the tabs in **tab_view** and directly serialize it into JSON to be sent as a message.
/// Helper function of `serialize_add_tabs()`.
/// Useful when sending first message of a newly established connection.
/// Caller takes ownership of return value and must free it.
const char* serialize_add_tabs_from_view(AdwTabView* tab_view);
/// Serialize an **array of AddTab** structs into JSON to be sent as a message.
/// Function takes ownership of **add_tabs** and frees it.
/// Caller takes ownership of return value and must free it.
const char* serialize_add_tabs(AddTab* add_tabs, size_t len);
/// Serialize a tab into JSON to be sent as a message.
/// Function takes ownership of **tab_idx** and frees it.
/// Caller takes ownership of return value and must free it.
const char* serialize_remove_tab(unsigned int tab_idx);
/// Serialize an **array of RenameTab** structs into JSON to be sent as a message.
/// Function takes ownership of **rename_tabs** and frees it.
/// Caller takes ownership of return value and must free it.
const char* serialize_rename_tabs(RenameTab* rename_tabs, size_t len);
/// Serialize a **DeleteContent** struct into JSON to be sent as a message.
/// Caller takes ownership of return value and must free it.
const char* serialize_delete_content(DeleteContent delete_content);
/// Serialize a **ReplaceContent** struct into JSON to be sent as a message.
/// Caller takes ownership of return value and must free it.
const char* serialize_replace_content(ReplaceContent delete_content);
/// Serialize a **InsertContent** struct into JSON to be sent as a message.
/// Caller takes ownership of return value and must free it.
const char* serialize_insert_content(InsertContent delete_content);


#endif // __UTIL_H__
