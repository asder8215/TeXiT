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

typedef struct {
    AdwTabPage* page;
    EditorBuffer* buffer;
} Page;

static void share_toggle_click(GtkToggleButton* toggle, GtkWindow* window);
/// *params* is malloc-ed by `share_toggle_click()`, which is a callback, so *params* must be freed by this function.
static void share_enable_response(AdwMessageDialog* dialog, const char* response, ShareEnableParams* params);
/// *params* is malloc-ed by `main_window()` (which essentially acts like `main()`) so it is allocated only once.
/// That same ptr should be freed only when the program terminates.
static void open_file_click(GtkButton* button, FileClickParams* params);
static void open_file_response(GtkNativeDialog* dialog, int response, AdwTabView* tab_view);
/// Refer to `open_file_click()` about *params*.
static void save_file_click(GtkButton* button, FileClickParams* params);

/// Appends a new page with a TextView to the TabView.
/// Sets the *title*, and a default icon.
/// Returns the newly created TabPage and its TextBuffer.
static Page new_tab_page(AdwTabView* tab_view, const char* title, const char* file_path);
/// Returns an alternative `Page` struct which references the AdwTabPage and TextBuffer of the current Tab.
/// Returns NULL if there are no tabs in the TabView.
static Page get_active_page(AdwTabView* tab_view);

/// Handles the close-page signal from closing a tab page. 
/// Will prompt the user with a message dialog whether they want to cancel
/// closing the tab, close the tab, or save the content from the tab if
/// there exist unsaved content.
static bool close_tab_page(AdwTabView* tab_view, AdwTabPage* page, GtkWindow* window);
/// Handles the response received from the close tab page message dialog.
static void close_unsaved_tab_response(AdwMessageDialog* dialog, GAsyncResult* result, FileClickParams* params);

void main_window(GtkApplication *app);
void main_window_destroy(GtkApplicationWindow* window, MainMalloced* params);

/// Sets up signal callbacks for the entries of the dialog in the *builder*.
/// Returns pointers to entries that will hold relevant values for hosting/connecting.
ShareDialogEntries share_dialog_entries(GtkBuilder* dialog_builder);

#endif // __GUI_H__
