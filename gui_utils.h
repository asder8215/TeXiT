#ifndef __GUI_UTILS_H__
#define __GUI_UTILS_H__

#include <gtk/gtk.h>

/// Get the widget with *name* in the descendants of *parent* using Breadth-First Search.
GtkWidget* get_widget_by_name(GtkWidget* parent, const char* name);
GtkWidget* widget_get_nth_child(GtkWidget* parent, unsigned int n);

#endif // __GUI_UTILS_H__