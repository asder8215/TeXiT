#include "gui.h"
#include <stdio.h>

static const char* SERVER_TOGGLE_ON_TITLE = "✔️ Share";
static const char* SERVER_TOGGLE_OFF_TITLE = "❌ Share";
typedef GtkWidget* Widget;

// Global variable for file name (will be changed later)
//GFile* file_name = NULL;

int newFile = 1;
char* filePath = NULL;

/// Handler for activating/deactivating the share feature. TODO: The *window* is the window to which the GtkDialog will be added to.
static void share_toggle_click(GtkToggleButton* toggle, gpointer window) {
    const char* label;
    if (gtk_toggle_button_get_active(toggle)) {
        // TODO: open dialog to configure
        label = SERVER_TOGGLE_ON_TITLE;
    } else {
        label = SERVER_TOGGLE_OFF_TITLE;
    }
    gtk_button_set_label(GTK_BUTTON(toggle), label);
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
        //file = gtk_file_chooser_get_file(GTK_FILE_CHOOSER(dialog));
		filePath = g_file_get_path(file);
		newFile = 0;
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

static void open_file_click(GtkButton* button, gpointer window) {	
	GtkFileChooserNative* file_chooser = gtk_file_chooser_native_new("Open File",
        window, GTK_FILE_CHOOSER_ACTION_OPEN, "Open", "Cancel"
    );

    gtk_native_dialog_set_modal(GTK_NATIVE_DIALOG(file_chooser), true);
    gtk_native_dialog_set_transient_for(GTK_NATIVE_DIALOG(file_chooser), GTK_WINDOW(window));
    g_signal_connect(file_chooser, "response", G_CALLBACK(open_file_response), NULL);
    gtk_native_dialog_show(GTK_NATIVE_DIALOG(file_chooser));
}


static void save_file_response(GtkNativeDialog* dialog, int response){
    GtkWindow* window = gtk_native_dialog_get_transient_for(dialog);
    // TODO: implement `get_widget_by_name("main-text-view")` and remove hardcoded solution.
    GtkTextBuffer* buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(
                                gtk_widget_get_first_child( // text_view
                                    gtk_widget_get_first_child( // scroller
                                        gtk_window_get_child(window) // window_box
                                    )
                                )
                            ));

	if(response == GTK_RESPONSE_ACCEPT){
        GFile* file = gtk_file_chooser_get_file(GTK_FILE_CHOOSER(dialog));
		filePath = g_file_get_path(file);
		GtkTextIter start, end;
		gtk_text_buffer_get_bounds(buffer, &start, &end);
		char* content = gtk_text_buffer_get_text(buffer, &start, &end, false);
		gsize length = gtk_text_buffer_get_char_count(buffer);
		if(g_file_replace_contents(file, content, length, 
		   NULL, false, G_FILE_CREATE_PRIVATE, NULL, NULL, NULL) == false){
			g_object_unref(file);
			exit(0);
		}
		g_object_unref(file);
	}
	g_object_unref(dialog);
}

static void save_file_click(GtkButton* button, gpointer window){
	GtkFileChooserNative* file_chooser = gtk_file_chooser_native_new("Save File", window, GTK_FILE_CHOOSER_ACTION_SAVE, 
																	 "Save", "Cancel");
	GtkFileChooser* chooser = GTK_FILE_CHOOSER(file_chooser);	
	
    GtkTextBuffer* buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(
                                gtk_widget_get_first_child( // text_view
                                    gtk_widget_get_first_child( // scroller
                                        gtk_window_get_child(window) // window_box
                                    )
                                )
                            ));
	
	if(newFile){
		gtk_file_chooser_set_current_name(chooser, "Untitled document.txt");
	}
	// If we're in an existing file, no need to bring up the save file dialog, just save the file.
	else{
		GFile* currFile = g_file_new_for_path((const char*) filePath);		
		
		GtkTextIter start, end;
		gtk_text_buffer_get_bounds(buffer, &start, &end);
		char* content = gtk_text_buffer_get_text(buffer, &start, 
												&end, false);
		gsize length = gtk_text_buffer_get_char_count(buffer);
		if(g_file_replace_contents(currFile, content, length, 
		   NULL, false, G_FILE_CREATE_PRIVATE, NULL, NULL, NULL) == false){
			g_object_unref(currFile);
			exit(0);
		}
		g_object_unref(currFile);
		return ;
		//gtk_file_chooser_set_file(chooser, currFile, NULL);
	}

    gtk_native_dialog_set_modal(GTK_NATIVE_DIALOG(file_chooser), true);
    gtk_native_dialog_set_transient_for(GTK_NATIVE_DIALOG(file_chooser), GTK_WINDOW(window));
    g_signal_connect(file_chooser, "response", G_CALLBACK(save_file_response), NULL);
    gtk_native_dialog_show(GTK_NATIVE_DIALOG(file_chooser));
}

void main_window(GtkApplication *app, gpointer user_data) {
    Widget window,
        headerbar,
            file_open,
			file_save,
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
    
	file_save = gtk_button_new();
	gtk_button_set_child(GTK_BUTTON(file_save), gtk_image_new_from_icon_name("edit-find-replace-symbolic"));
	g_signal_connect(file_save, "clicked", G_CALLBACK(save_file_click), window);
	gtk_header_bar_pack_start(GTK_HEADER_BAR(headerbar), file_save);	

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

GtkWidget* get_widget_by_name(GtkWidget* parent, const char* name) {
    printf("Not implemented, using hardcoded solution.\n");
    exit(0);
}
