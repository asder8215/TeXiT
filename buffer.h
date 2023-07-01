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
EditorBuffer* editor_buffer_new(const char* file_path, AdwTabPage* tab_page);

const char* editor_buffer_get_file_path(EditorBuffer* self);

/// Get the tab page which this Buffer belongs to.
AdwTabPage* editor_buffer_get_page(EditorBuffer* self);

/// sets and gets the edited value of the buffer
/// Wish edited could just be pub :(
bool editor_buffer_get_edited(EditorBuffer* self);
void editor_buffer_set_edited(EditorBuffer* self, bool value);

/// Saves the contents of *self* to the file at *self.file_path*, overwriteing the file.
/// Gets the *current_page* (the one this buffer is in) from **tab_view** and sets its title.
/// The FilePicker dialog is set trainsient for *parent_window*, which could be NULL.
/// **close_tab** is `true` if this function is called from a `AdwTabView::close-page` callback,
/// so this function will finish closing the page after file is saved.
void editor_buffer_save(EditorBuffer* self, AdwTabView* tab_view, GtkWindow* parent_window, bool close_tab);

G_END_DECLS

#endif // __BUFFER_H__
