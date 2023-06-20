#ifndef __GUI_H__
#define __GUI_H__

#include <gtk/gtk.h>
#include <adwaita.h>

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
    // Reference to the tabview to create a new tab with the file content.
    AdwTabView* tab_view;
} FileClickParams;

/// Things that were heap-allocated in `main_window()` and must be freed when program terminates.
typedef struct {
    FileClickParams* file_click_params;
} MainMalloced;

typedef struct {
    AdwTabPage* page;
    GtkTextBuffer* buffer;
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
/// Writes the Buffer of the current TabPage, and sets the Tab title if was a new file.
static void save_file_response(GtkNativeDialog* dialog, int response, FileClickParams* params);

/// Appends a new page with a TextView to the TabView.
/// Sets the *title*, and a default icon.
/// Returns the newly created TabPage and its TextBuffer.
static Page new_tab_page(AdwTabView* tab_view, const char* title, const char* filePath);
/// Returns an alternative `Page` struct which references the AdwTabPage and TextBuffer of the current Tab.
static Page get_active_page(AdwTabView* tab_view);

void main_window(GtkApplication *app);
void main_window_destroy(GtkApplicationWindow* window, MainMalloced* params);

/// Creates the inner UI for the dialog that allows user to activate sharing.
/// Returns pointers to entries that will hold relevant values for hosting/connecting.
ShareDialogEntries set_share_dialog_child(AdwMessageDialog* dialog);

#endif // __GUI_H__
