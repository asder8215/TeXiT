#ifndef __GUI_H__
#define __GUI_H__

#include <gtk/gtk.h>

void main_window(GtkApplication *app, gpointer user_data);
/// Creates the UI for the dialog that allows user to activate sharing.
void share_dialog(GtkBox* dialog_content_area);
GtkWidget* get_widget_by_name(GtkWidget* parent, const char* name);

#endif // __GUI_H__