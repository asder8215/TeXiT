#include "gui.h"
#include <stdio.h>

#define SHARE_RESPONSE_HOST 1
#define SHARE_RESPONSE_CONNNECT 2

static const char* SERVER_TOGGLE_ON_TITLE = "✔️ Share";
static const char* SERVER_TOGGLE_OFF_TITLE = "❌ Share";
typedef GtkWidget* Widget;

static void share_toggle_click(GtkToggleButton* toggle, gpointer window);
static void share_enable_response(GtkDialog* dialog, int response);
static void open_file_click(GtkButton* button, gpointer window);
static void open_file_response(GtkNativeDialog* dialog, int response);

/// Handler for activating/deactivating the share feature. A GtkDialog will be crated on top of *window*.
static void share_toggle_click(GtkToggleButton* toggle, gpointer window) {
    const char* label;
    if (gtk_toggle_button_get_active(toggle)) {
        // TODO: open dialog to configure
        GtkDialog* dialog = gtk_dialog_new_with_buttons("Start Sharing", GTK_WINDOW(window),
                                GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                "Host", SHARE_RESPONSE_HOST,
                                "Connect", SHARE_RESPONSE_HOST, NULL
                            );
        share_dialog(gtk_dialog_get_content_area(dialog));
        gtk_window_set_resizable(GTK_WINDOW(dialog), false);
        g_signal_connect(dialog, "response", G_CALLBACK(share_enable_response), NULL);
        gtk_widget_show(dialog);
        // TODO: deactivate toggle button. `share_enable_response()` should activate the toggle button.
    } else {
        label = SERVER_TOGGLE_OFF_TITLE;
        gtk_button_set_label(GTK_BUTTON(toggle), label);
    }
}

/// Set up sharing connection (either hosting or connecting).
static void share_enable_response(GtkDialog* dialog, int response) {
    
}

static void open_file_click(GtkButton* button, gpointer window) {
    GtkFileChooserNative* file_chooser = gtk_file_chooser_native_new("Open File",
        window, GTK_FILE_CHOOSER_ACTION_OPEN, "Open", "Cancel"
    );
    gtk_native_dialog_set_modal(GTK_NATIVE_DIALOG(file_chooser), true);
    gtk_native_dialog_set_transient_for(GTK_NATIVE_DIALOG(file_chooser), GTK_WINDOW(window));
    g_signal_connect(file_chooser, "response", G_CALLBACK(open_file_response), NULL);
    gtk_native_dialog_show(GTK_NATIVE_DIALOG(file_chooser));
}

static void open_file_response(GtkNativeDialog* dialog, int response) {
    GtkWindow* window = gtk_native_dialog_get_transient_for(dialog);
    // TODO: implement `get_widget_by_name("main-text-view")` and remove hardcoded solution.
    GtkTextBuffer* buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(
                                gtk_widget_get_first_child( // text_view
                                    gtk_widget_get_first_child( // scroller
                                        gtk_window_get_child(window) // window_box
                                    )
                                )
                            ));

    if (response == GTK_RESPONSE_ACCEPT) {
        GFile* file = gtk_file_chooser_get_file(GTK_FILE_CHOOSER(dialog));
        printf("Open File: %s\n", g_file_get_path(file));

        char* content;
        gsize length;
        GError* error;
        if (g_file_load_contents(file, NULL, &content, &length, NULL, &error)) {
            gtk_text_buffer_set_text(buffer, (const char*)content, -1);
        } else {
            // TODO: Print error
            exit(0);
        }

        g_object_unref (file);
    }

    g_object_unref(dialog);
}


void main_window(GtkApplication *app, gpointer user_data) {
    Widget window,
        headerbar,
            file_open,
            // folder_open,
            share_toggle,
        window_box,
            scroller,
                text_view, // main-text-view
            action_bar;

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Window");
    headerbar = gtk_header_bar_new();
    gtk_window_set_titlebar(GTK_WINDOW(window), headerbar);

    file_open = gtk_button_new();
    gtk_button_set_child(GTK_BUTTON(file_open), gtk_image_new_from_icon_name("text-x-generic-symbolic"));
    g_signal_connect(file_open, "clicked", G_CALLBACK(open_file_click), window);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(headerbar), file_open);
    // folder_open = gtk_button_new();
    // gtk_button_set_child(GTK_BUTTON(folder_open), gtk_image_new_from_icon_name("folder-symbolic"));
    // g_signal_connect(folder_open, "clicked", G_CALLBACK(open_folder_click), window);
    // gtk_header_bar_pack_start(GTK_HEADER_BAR(headerbar), folder_open);
    share_toggle = gtk_toggle_button_new_with_label(SERVER_TOGGLE_OFF_TITLE);
    g_signal_connect(share_toggle, "toggled", G_CALLBACK(share_toggle_click), window);
    gtk_header_bar_pack_end(GTK_HEADER_BAR(headerbar), share_toggle);

    text_view = gtk_text_view_new();
    gtk_widget_set_name(text_view, "main-text-view");
    scroller = gtk_scrolled_window_new();
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroller), text_view);
    gtk_widget_set_vexpand(scroller, true);
    gtk_widget_set_size_request(scroller, 400, 200);

    action_bar = gtk_action_bar_new();
    gtk_widget_set_vexpand(action_bar, false);
    gtk_action_bar_pack_start(GTK_ACTION_BAR(action_bar), gtk_label_new("Share: Not connected"));

    window_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_append(GTK_BOX(window_box), scroller);
    gtk_box_append(GTK_BOX(window_box), action_bar);
    gtk_window_set_child(GTK_WINDOW(window), window_box);

    gtk_window_present(GTK_WINDOW(window));
}

/// Creates the UI for the dialog that allows user to activate sharing.
void share_dialog(GtkBox* dialog_content_area) {
    gtk_box_append(dialog_content_area, gtk_label_new("Host session"));
    gtk_box_append(dialog_content_area, gtk_entry_new());
    gtk_box_append(dialog_content_area, gtk_label_new("Or:"));
    gtk_box_append(dialog_content_area, gtk_label_new("Connect to an exisitng session"));
    gtk_box_append(dialog_content_area, gtk_entry_new());
}

GtkWidget* get_widget_by_name(GtkWidget* parent, const char* name) {
    printf("Not implemented, using hardcoded solution.\n");
    exit(0);
}
