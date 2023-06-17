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

/// Parameters passed to both *open* and *save* file buttons.
typedef struct {
    /// Reference to the main-window so it can attach to it as modal.
    GtkWindow* window;
    /// Reference to the main-text-buffer for text-editing.
    GtkTextBuffer* buffer;
} FileClickParams;

/// Things that were heap-allocated in `main_window()` and must be freed when program terminates.
typedef struct {
    FileClickParams* file_click_params;
} MainMalloced;

static void share_toggle_click(GtkToggleButton* toggle, GtkWindow* window);
/// *params* is malloc-ed by `share_toggle_click()`, which is a callback, so *params* must be freed by this function.
static void share_enable_response(GtkDialog* dialog, int response, ShareEnableParams* params);
/// *params* is malloc-ed by `main_window()` (which essentially acts like `main()`) so it is allocated only once.
/// That same ptr should be freed only when the program terminates.
static void open_file_click(GtkButton* button, FileClickParams* params);
static void open_file_response(GtkNativeDialog* dialog, int response, GtkTextBuffer* buffer);
/// Refer to `open_file_click()` about *params*.
static void save_file_click(GtkButton* button, FileClickParams* params);
static void save_file_response(GtkNativeDialog* dialog, int response, GtkTextBuffer* buffer);

void main_window(GtkApplication *app);
void main_window_destroy(GtkApplicationWindow* window, MainMalloced* params);
/// Creates the UI for the dialog that allows user to activate sharing.
/// Returns pointers to entries that will hold relevant values for hosting/connecting.
ShareDialogEntries share_dialog(GtkBox* dialog_content_area);

#endif // __GUI_H__