#ifndef __GUI_H__
#define __GUI_H__

#include <gtk/gtk.h>
#include <adwaita.h>
#include "buffer.h"

typedef GtkWidget* Widget;

typedef struct {
    GtkEditable* host_port;
    GtkEditable* connect_ip;
    GtkEditable* connect_port;
} ShareDialogEntries;

typedef struct {
    GtkButton* toggle;
    ShareDialogEntries entries;
} ShareEnableParams;

/// Parameters passed to both *open* and *save* file buttons.
typedef struct {
    /// Reference to the main-window so it can attach to it as modal.
    GtkWindow* window;
    /// Reference to the tabview to create a new tab with the file content.
    AdwTabView* tab_view;
    /// Reference to the overlay to show Toasts.
    AdwToastOverlay* toast_overlay;
} FileClickParams;

/// Things that were heap-allocated in `main_window()` and must be freed when program terminates.
typedef struct {
    FileClickParams* file_click_params;
} MainMalloced;

/*
typedef struct {
    AdwTabPage* page;
    EditorBuffer* buffer;
} Page;
*/

static void share_toggle_click(GtkToggleButton* toggle, GtkWindow* window);
/// *params* is malloc-ed by `share_toggle_click()`, which is a callback, so *params* must be freed by this function.
static void share_enable_response(AdwMessageDialog* dialog, const char* response, ShareEnableParams* params);
/// *params* is malloc-ed by `main_window()` (which essentially acts like `main()`) so it is allocated only once.
/// That same ptr should be freed only when the program terminates.
static void open_file_click(GtkButton* button, FileClickParams* params);
static void open_file_response(GtkNativeDialog* dialog, int response, AdwTabView* tab_view);

void main_window(GtkApplication *app);
void main_window_destroy(GtkApplicationWindow* window, MainMalloced* params);

/// Sets up signal callbacks for the entries of the dialog in the *builder*.
/// Returns pointers to entries that will hold relevant values for hosting/connecting.
ShareDialogEntries share_dialog_entries(GtkBuilder* dialog_builder);

#endif // __GUI_H__
