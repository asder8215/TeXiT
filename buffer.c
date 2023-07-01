#include "buffer.h"
#include "gui.h"
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

static void editor_buffer_changed(EditorBuffer* buffer){
    if(!buffer->edited){
        const char* tab_page_title = adw_tab_page_get_title(buffer->tab_page);
        char* modified_title = NULL;
        int len_tab_page_title = strlen(tab_page_title);
        modified_title = malloc(len_tab_page_title + 2);
        strcpy(modified_title, tab_page_title);
        strcat(modified_title, "*");
    
        adw_tab_page_set_title(buffer->tab_page, (const char*) modified_title);
        buffer->edited = true;

        free(modified_title);
    }
}

static void editor_buffer_init(EditorBuffer* self) {
    self->edited = false;
    self->file_path = NULL;
}

EditorBuffer* editor_buffer_new(const char* file_path, AdwTabPage* tab_page) {
    if (file_path != NULL)
        file_path = strdup(file_path);
    
    EditorBuffer* buffer = g_object_new(EDITOR_TYPE_BUFFER, NULL);
    buffer->edited = false;
    buffer->file_path = file_path;
    buffer->tab_page = tab_page;
    g_signal_connect(GTK_TEXT_BUFFER(buffer), "changed", G_CALLBACK(editor_buffer_changed), NULL);
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

static void save_response(GtkFileDialog* dialog, GAsyncResult* result, SaveResponseParams* params) {
    GFile* file = gtk_file_dialog_save_finish(dialog, result, NULL);
    AdwTabPage* current_page = adw_tab_view_get_selected_page(params->tab_view);
    
    if (file == NULL) {
        // User cancelled FileDialog
        // Do not close tab
        if (params->close_tab)
            adw_tab_view_close_page_finish(params->tab_view, current_page, false);
    } else {
        // File was saved
        write_file(file, &params->buffer->parent);
        // Set tab title
        adw_tab_page_set_title(current_page, g_file_get_basename(file));
        // Set Buffer file_path
        params->buffer->file_path = g_file_get_path(file);
        
        // Close tab
        if (params->close_tab)
            adw_tab_view_close_page_finish(params->tab_view, current_page, true);
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
        gtk_file_dialog_save(dialog, parent_window, NULL, (GAsyncReadyCallback)(save_response), params);
        return;
    }

    // Using an exisitng file, overwrite it.
    GFile* file = g_file_new_for_path(self->file_path);
    write_file(file, &self->parent);
}
