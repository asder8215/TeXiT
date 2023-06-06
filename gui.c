#include "gui.h"
#include <stdio.h>

static const char* SERVER_TOGGLE_ON_TITLE = "✔️ Share";
static const char* SERVER_TOGGLE_OFF_TITLE = "❌ Share";
typedef GtkWidget* Widget;

/// Handler for activating/deactivating the share feature. TODO: The *window* is the window to which the GtkDialog will be added to.
static void share_toggle_click(GtkToggleButton* toggle, gpointer window) {
    const char* label;
    if (gtk_toggle_button_get_active(toggle)) {
        label = SERVER_TOGGLE_ON_TITLE;
    } else {
        label = SERVER_TOGGLE_OFF_TITLE;
    }
    gtk_button_set_label(GTK_BUTTON(toggle), label);
}

void main_window(GtkApplication *app, gpointer user_data) {
    Widget window,
        headerbar,
            share_toggle,
        window_box,
            scroller,
                text_view,
            action_bar;

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Window");
    headerbar = gtk_header_bar_new();
    gtk_window_set_titlebar(GTK_WINDOW(window), headerbar);

    share_toggle = gtk_toggle_button_new_with_label(SERVER_TOGGLE_OFF_TITLE);
    g_signal_connect(share_toggle, "toggled", G_CALLBACK(share_toggle_click), window);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(headerbar), share_toggle);

    text_view = gtk_text_view_new();
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
