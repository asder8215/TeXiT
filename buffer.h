#ifndef __BUFFER_H__
#define __BUFFER_H__

#include <gtk/gtk.h>
#include <adwaita.h>

G_BEGIN_DECLS

// If only this could be done for AdwTabPage. But alas, it is final.
#define EDITOR_TYPE_BUFFER (editor_buffer_get_type())
G_DECLARE_FINAL_TYPE(EditorBuffer, editor_buffer, EDITOR, BUFFER, GtkTextBuffer)

/// *file_path* is copied by this function, so it is only borrowed. Can be NULL.
EditorBuffer* editor_buffer_new(const char* file_path);
const char* editor_buffer_get_file_path(EditorBuffer* self);
bool editor_buffer_get_edited(EditorBuffer* self);
/// Saves the contents of *self* to the file at *self.file_path*, overwriteing the file.
/// Sets the title of the *current_page*, which is the page the buffer is in.
/// The FilePicker dialog is set trainsient for *parent_window*, which could be NULL.
void editor_buffer_save(EditorBuffer* self, AdwTabPage* current_page, GtkWindow* parent_window);

G_END_DECLS

// TODO: acc GtkBuffer::changed signal to set edited to `true` when user types

#endif // __BUFFER_H__
