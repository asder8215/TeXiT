#include "buffer.h"
#include "tab-page.h"
#include "gui.h"
#include "server.h"
#include <string.h>

struct _EditorBuffer {
    GtkTextBuffer parent;
    // field not used for now
    bool edited;
    /// Can be NULL.
    const char* file_path;
    // associated tab_page for buffer
    AdwTabPage* tab_page;
};
G_DEFINE_TYPE(EditorBuffer, editor_buffer, GTK_TYPE_TEXT_BUFFER)

/// Destructor
static void editor_buffer_finalize(GObject *gobject) {
    EditorBuffer* buffer = EDITOR_BUFFER(gobject);
    free((void*)buffer->file_path);
    // Always chain up to the parent class; as with dispose(), finalize() is guaranteed to exist on the parent's class virtual function table
    G_OBJECT_CLASS(editor_buffer_parent_class)->finalize(gobject);
}

static void editor_buffer_class_init(EditorBufferClass* class) {
    GObjectClass* object_class = G_OBJECT_CLASS(class);
    // Setup destructor to free file_path
    object_class->finalize = editor_buffer_finalize;

    // Leave here just in case, but will not be used.
    // // Install properties
    // GParamSpec* edited_param_spec = g_param_spec_boolean(
    //     "edited", NULL,
    //     "Whether the user typed something into the Buffer.",
    //     false, G_PARAM_READABLE | G_PARAM_CONSTRUCT
    // );
    // g_object_class_install_property(object_class, 1, edited_param_spec);
    // free(edited_param_spec);
    // GParamSpec* file_path_param_spec = g_param_spec_string(
    //     "file-path", NULL,
    //     "The path of the file the Buffer will write to when saved.\nWill present a file-picker dialog when trying to save and the value is NULL.",
    //     NULL, G_PARAM_READABLE | G_PARAM_CONSTRUCT
    // );
    // g_object_class_install_property(object_class, 2, file_path_param_spec);
    // free(file_path_param_spec);
}

/// Callback called whenever something changes in the TextBuffer,
/// regardless of it was made by a Client or by the User.
static void editor_buffer_changed(EditorBuffer* buffer, AdwTabView* tab_view) {
    if (!buffer->edited) {
        adw_tab_page_set_indicator_icon(buffer->tab_page, g_themed_icon_new("media-record-symbolic"));
        buffer->edited = true;
    }
}
/// When the user types something, the change must be sent to all of its clients.
static void user_typed(EditorBuffer* buffer, AdwTabView* tab_view) {
    TabContent tab_content;
    tab_content.tab_idx = adw_tab_view_get_page_position(tab_view, buffer->tab_page);
    tab_content.content = editor_buffer_get_content(buffer);
    server_change_tab_content(tab_content);
}


static void editor_buffer_init(EditorBuffer* self) {
    self->edited = false;
    self->file_path = NULL;
}

EditorBuffer* editor_buffer_new(const char* file_path, AdwTabPage* tab_page, AdwTabView* tab_view) {
    if (file_path != NULL)
        file_path = strdup(file_path);
    
    EditorBuffer* buffer = g_object_new(EDITOR_TYPE_BUFFER, NULL);
    buffer->edited = false;
    buffer->file_path = file_path;
    buffer->tab_page = tab_page;
    g_signal_connect(GTK_TEXT_BUFFER(buffer), "changed", G_CALLBACK(editor_buffer_changed), tab_view);
    g_signal_connect(GTK_TEXT_BUFFER(buffer), "end-user-action", G_CALLBACK(user_typed), tab_view);
    return buffer;
}

const char* editor_buffer_get_file_path(EditorBuffer* self) {
    // Why can't do this outside?
    return self->file_path;
}
bool editor_buffer_get_edited(EditorBuffer* self) {
    return self->edited;
}

void editor_buffer_set_edited(EditorBuffer* self, bool value){
    self->edited = value;
}

AdwTabPage* editor_buffer_get_page(EditorBuffer* self){
    return self->tab_page;
}

const char* editor_buffer_get_content(EditorBuffer* self) {
    GtkTextBuffer* buffer = GTK_TEXT_BUFFER(self);
    GtkTextIter start;
    GtkTextIter end;
    gtk_text_buffer_get_start_iter(buffer, &start);
    gtk_text_buffer_get_end_iter(buffer, &end);
    return gtk_text_buffer_get_text(buffer, &start, &end, true);
}

// Takes ownership of *file*. *buffer* is a reference.
static void write_file(GFile* file, GtkTextBuffer* buffer) {
    // Fetching the current content in the file
    GError* error = NULL;
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(buffer, &start, &end);
    const char* content = gtk_text_buffer_get_text(buffer, &start, &end, false);
    
    // Overwriting the file content with the written content in the application
    // Error message will pop up if replacing content fails
    if(!g_file_replace_contents(
        file, content, strlen(content),
        NULL, false, G_FILE_CREATE_NONE,
        NULL, NULL, &error
    )) {
        printf("Error saving file (\"%s\"): %s", g_file_get_path(file), error->message);
        g_object_unref(error);
    }

    g_object_unref(file);
}

typedef struct {
    AdwTabView* tab_view;
    EditorBuffer* buffer;
    bool close_tab;
} SaveResponseParams;

static void new_file_save_response(GtkFileDialog* dialog, GAsyncResult* result, SaveResponseParams* params) {
    GFile* file = gtk_file_dialog_save_finish(dialog, result, NULL);
    
    if (file == NULL) {
        // User cancelled FileDialog
        // Do not close tab
        if (params->close_tab)
            adw_tab_view_close_page_finish(params->tab_view, params->buffer->tab_page, false);
    } else {
        // File was saved
        write_file(file, GTK_TEXT_BUFFER(params->buffer));
        // Set tab title
        adw_tab_page_set_title(params->buffer->tab_page, g_file_get_basename(file));
        // Set Buffer file_path
        params->buffer->file_path = g_file_get_path(file);
        params->buffer->edited = false;
        // Remove unsaved indicator
        adw_tab_page_set_indicator_icon(params->buffer->tab_page, NULL);
        
        // Close tab
        if (params->close_tab)
            adw_tab_view_close_page_finish(params->tab_view, params->buffer->tab_page, true);
        if (adw_tab_view_get_n_pages(params->tab_view) == 0)
            gtk_widget_set_visible(GTK_WIDGET(params->tab_view), false);
    }

    g_object_unref(dialog);
    free(params);
}

void editor_buffer_save(EditorBuffer* self, AdwTabView* tab_view, GtkWindow* parent_window, bool close_tab) {
    if (self->file_path == NULL) {
        // We're in a completely new file, ask user where to save.
        GtkFileDialog* dialog = gtk_file_dialog_new();
        gtk_file_dialog_set_modal(dialog, true);
        gtk_file_dialog_set_title(dialog, "Save File");
        gtk_file_dialog_set_initial_name(dialog, "Untitled document.txt");
        // Later: If in a folder context, set initial_folder to it.

        // Freed in `save_response()`
        SaveResponseParams* params = malloc(sizeof(SaveResponseParams));
        params->tab_view = tab_view;
        params->buffer = self;
        params->close_tab = close_tab;
        gtk_file_dialog_save(dialog, parent_window, NULL, (GAsyncReadyCallback)(new_file_save_response), params);
        return;
    }

    // Using an exisitng file, overwrite it.
    GFile* file = g_file_new_for_path(self->file_path);
    write_file(file, GTK_TEXT_BUFFER(self));
    self->edited = false;
    // Remove unsaved indicator
    adw_tab_page_set_indicator_icon(self->tab_page, NULL);
}
