#ifndef __UTIL_H__
#define __UTIL_H__

#include "arraylist.h"
#include <gdk/gdk.h>
#include <stdbool.h>
#include <json.h>

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
/// Should be malloced and freed.
typedef struct {
    unsigned int tab_idx;
    const char* title;
    const char* content;
} AddTab;
AddTab* add_tab_new(unsigned int tab_idx, const char* title, const char* content);
void add_tab_free(AddTab* self);

/// Has ownership of title.
/// Should be malloced and freed.
typedef struct {
    unsigned int tab_idx;
    const char* title;
} RenameTab;
RenameTab* rename_tab_new(unsigned int tab_idx, const char* title);
void rename_tab_free(RenameTab* self);

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
ReplaceContent* replace_content_new(unsigned int tab_idx, size_t start_byte, size_t byte_length, const char* content);
void replace_content_free(ReplaceContent* self);

/// Has ownership of content.
/// Should be malloced and freed.
typedef struct {
    unsigned int tab_idx;
    size_t start_byte;
    const char* content;
} InsertContent;
InsertContent* insert_content_new(unsigned int tab_idx, size_t start_byte, const char* content);
void insert_content_free(InsertContent* self);

typedef enum {
    MSG_T_ADD_TABS,
    MSG_T_RM_TABS,
    MSG_T_RENAME_TABS,
    MSG_T_DEL_CONTENT,
    MSG_T_REPLACE_CONTENT,
    MSG_T_INSERT_CONTENT
} MessageType;

/// Deserialize a json list containing AddTab structs
array_list deserialize_add_tabs(const char* json);
/// Deserialize a json list containing Tab Indexes
array_list deserialize_remove_tabs(const char* json);
/// Deserialize a json list containing RemoveTab structs
array_list deserialize_rename_tabs(const char* json);
/// Can be NULL.
DeleteContent* deserialize_delete_content(const char* json);
/// Can be NULL.
ReplaceContent* deserialize_replace_content(const char* json);
/// Can be NULL.
InsertContent* deserialize_insert_content(const char* json);

#endif // __UTIL_H__