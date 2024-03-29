#ifndef __BUFFER_H__
#define __BUFFER_H__

#include <gtk/gtk.h>
#include <adwaita.h>

G_BEGIN_DECLS

// If only this could be done for AdwTabPage. But alas, it is final.
#define EDITOR_TYPE_BUFFER (editor_buffer_get_type())
G_DECLARE_FINAL_TYPE(EditorBuffer, editor_buffer, EDITOR, BUFFER, GtkTextBuffer)

/// *file_path* is copied by this function, so it is only borrowed. Can be NULL.
/// *tab_page* is a reference (Can't be NULL) to the `AdwTabPage` to which this buffer is in.
EditorBuffer* editor_buffer_new(const char* file_path, AdwTabPage* tab_page, AdwTabView* tab_view);

const char* editor_buffer_get_file_path(EditorBuffer* self);

/// Get the tab page which this Buffer belongs to.
AdwTabPage* editor_buffer_get_page(EditorBuffer* self);

/// sets and gets the edited value of the buffer
/// Wish edited could just be pub :(
bool editor_buffer_get_edited(EditorBuffer* self);
void editor_buffer_set_edited(EditorBuffer* self, bool value);

/// Convenience function to get the content of the inner GtkTextBuffer using `gtk_text_buffer_get_text`.
/// Caller takes ownership or return value and must `free` it.
const char* editor_buffer_get_content(EditorBuffer* self);

/// Saves the contents of *self* to the file at *self.file_path*, overwriting the file.
/// Sets the title of *self->tab_page* to the name of the file that was saved, or closes it if *close_tab* is true.
/// **close_tab** is `true` if this function is called from a `AdwTabView::close-page` callback,
/// The FilePicker dialog is set trainsient for *parent_window*, which could be NULL.
void editor_buffer_save(EditorBuffer* self, AdwTabView* tab_view, GtkWindow* parent_window, bool close_tab);

G_END_DECLS

#endif // __BUFFER_H__
