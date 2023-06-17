#ifndef __GUI_H__
#define __GUI_H__

#include <gtk/gtk.h>

typedef GtkWidget* Widget;

typedef struct {
    GtkEntry* host_port;
    GtkEntry* connect_ip;
    GtkEntry* connect_port;
} ShareDialogEntries;

typedef struct {
    GtkButton* toggle;
    ShareDialogEntries entries;
} ShareEnableParams;

static void share_toggle_click(GtkToggleButton* toggle, gpointer window);
static void share_enable_response(GtkDialog* dialog, int response, ShareEnableParams* params);
static void open_file_click(GtkButton* button, gpointer window);
static void open_file_response(GtkNativeDialog* dialog, int response);
static void save_file_click(GtkButton* button, gpointer window);
static void save_file_response(GtkNativeDialog* dialog, int response);

void main_window(GtkApplication *app, gpointer user_data);
/// Creates the UI for the dialog that allows user to activate sharing.
/// Returns pointers to entries that will hold relevant values for hosting/connecting.
ShareDialogEntries share_dialog(GtkBox* dialog_content_area);

#endif // __GUI_H__