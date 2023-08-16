#ifndef __GUI_H__
#define __GUI_H__

#include <gtk/gtk.h>
#include <adwaita.h>

static const char* TOGGLE_LABEL_OFF = "❌ Share";
static const char* TOGGLE_LABEL_HOSTING = "✔️ Hosting";
static const char* TOGGLE_LABEL_CONNECTED = "✔️ Connected";

extern GtkWindow* window;
extern AdwToastOverlay* toast_overlay;
extern AdwTabView* tab_view;
extern AdwTabBar* tabbar;
extern GtkLabel* window_label;

extern GtkToggleButton* share_toggle;
extern GtkButton* file_new_btn;
extern GtkButton* file_open_btn;
extern GtkButton* file_save_btn;

typedef GtkWidget* Widget;

typedef struct {
    GtkEditable* host_port;
    GtkEditable* connect_ip;
    GtkEditable* connect_port;
} ShareDialogEntries;

// Signal callbacks for "res/main-window.blp".
void share_toggle_click(GtkToggleButton* toggle, gpointer _);
void new_file_click(GtkButton* button, gpointer _);
void open_file_click(GtkButton* button, gpointer _);
void save_file_click(GtkButton* button, gpointer _);
gboolean main_window_destroy(AdwApplicationWindow* window, gpointer _);

void main_window(AdwApplication *app);

/// Sets up signal callbacks for the entries of the dialog in the *builder*.
/// Returns pointers to entries that will hold relevant values for hosting/connecting.
ShareDialogEntries share_dialog_entries(GtkBuilder* dialog_builder);

#endif // __GUI_H__
