#ifndef __GUI_H__
#define __GUI_H__

#include <gtk/gtk.h>
#include <adwaita.h>

static const char* TOGGLE_LABEL_OFF = "❌ Share";
static const char* TOGGLE_LABEL_HOSTING = "✔️ Hosting";
static const char* TOGGLE_LABEL_CONNECTED = "✔️ Connected";

typedef GtkWidget* Widget;

typedef struct {
    GtkEditable* host_port;
    GtkEditable* connect_ip;
    GtkEditable* connect_port;
} ShareDialogEntries;

typedef struct {
    GtkButton* file_new;
    GtkButton* file_save;
    GtkButton* file_open;
} FileButtons;

typedef struct {
    GtkWindow* window;
    FileButtons* file_buttons;
    GtkLabel* label;
    AdwTabBar* tabbar;
    AdwToastOverlay* toast_overlay;
    AdwTabView* tab_view;
} ShareClickParams;

typedef struct {
    GtkWindow* window;
    FileButtons* file_buttons;
    GtkLabel* label;
    AdwTabBar* tabbar;
    GtkToggleButton* toggle;
    AdwToastOverlay* toast_overlay;
    AdwTabView* tab_view;
    ShareDialogEntries entries;
} ShareEnableParams;

/// Parameters passed to both *open* and *save* file buttons.
typedef struct {
    /// Reference to the main-window so it can attach to it as modal.
    GtkWindow* window;
    /// Reference to the initial label when no tabs exist on the editor.
    GtkLabel* label;
    /// Reference to the tabbar to set visibility for it.
    AdwTabBar* tabbar;
    /// Reference to the tabview to create a new tab with the file content.
    AdwTabView* tab_view;
    /// Reference to the overlay to show Toasts.
    AdwToastOverlay* toast_overlay;
} FileClickParams;

/// Things that were heap-allocated in `main_window()` and must be freed when program terminates.
typedef struct {
    FileClickParams* file_click_params;
    ShareClickParams* share_click_params;
    FileButtons* file_buttons;
} MainMalloced;

static void share_toggle_click(GtkToggleButton* toggle, ShareClickParams* params);
/// *params* is malloc-ed by `share_toggle_click()`, which is a callback, so *params* must be freed by this function.
static void share_enable_response(AdwMessageDialog* dialog, const char* response, ShareEnableParams* params);

void main_window(AdwApplication *app);
gboolean main_window_destroy(AdwApplicationWindow* window, MainMalloced* params);

/// Sets up signal callbacks for the entries of the dialog in the *builder*.
/// Returns pointers to entries that will hold relevant values for hosting/connecting.
ShareDialogEntries share_dialog_entries(GtkBuilder* dialog_builder);

#endif // __GUI_H__
