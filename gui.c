#include "gui.h"
#include <stdio.h>
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#define SHARE_RESPONSE_HOST 1
#define SHARE_RESPONSE_CONNNECT 2

static const char* SERVER_TOGGLE_ON_TITLE = "✔️ Share";
static const char* SERVER_TOGGLE_OFF_TITLE = "❌ Share";

// Global variable for file name (will be changed later)
static char* filePath = NULL;

/// Handler for activating/deactivating the share feature. A GtkDialog will be crated on top of *window*.
static void share_toggle_click(GtkToggleButton* toggle, GtkWindow* window) {
    const char* label;
    if (gtk_toggle_button_get_active(toggle)) {
        // Freed by `share_enable_response()`.
        GtkDialog* dialog = GTK_DIALOG(gtk_dialog_new_with_buttons("Start Sharing", GTK_WINDOW(window),
                                GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                "Host", SHARE_RESPONSE_HOST,
                                "Connect", SHARE_RESPONSE_CONNNECT, NULL
                            ));
        ShareDialogEntries entries = share_dialog(GTK_BOX(gtk_dialog_get_content_area(dialog)));
        // C moment :( Why must it be done like this
        // Freed by `share_enable_response()`.
        ShareEnableParams* params = malloc(sizeof(ShareEnableParams));
        params->toggle = GTK_BUTTON(toggle);
        params->entries = entries;

        gtk_window_set_resizable(GTK_WINDOW(dialog), false);
        g_signal_connect(dialog, "response", G_CALLBACK(share_enable_response), params);
        gtk_window_present(GTK_WINDOW(dialog));
        // Deactivate toggle button. `share_enable_response()` should activate the toggle button if setup was successful.
        gtk_toggle_button_set_active(toggle, false);
    } else {
        label = SERVER_TOGGLE_OFF_TITLE;
        gtk_button_set_label(GTK_BUTTON(toggle), label);
    }
}

/// Set up sharing connection (either hosting or connecting).
static void share_enable_response(GtkDialog* dialog, int response, ShareEnableParams* params) {
    if (response == SHARE_RESPONSE_HOST) {
        const char* port = gtk_entry_buffer_get_text(gtk_entry_get_buffer(params->entries.host_port));
        printf("Host with port %s\n", port);
    } else if (response == SHARE_RESPONSE_CONNNECT) {
        const char* ip = gtk_entry_buffer_get_text(gtk_entry_get_buffer(params->entries.connect_ip));
        const char* port = gtk_entry_buffer_get_text(gtk_entry_get_buffer(params->entries.connect_port));
        printf("Connect to %s with port %s\n", ip, port);
    } else {
        return;
    }
    // TODO: only change label if server start was successful.
    gtk_button_set_label(params->toggle, SERVER_TOGGLE_ON_TITLE);
    gtk_window_destroy(GTK_WINDOW(dialog));
    free(params);
}


static void new_file_click(GtkButton* button, FileClickParams* params) {
    printf("New File\n");
    new_tab_page(params->tab_view);
}


static void open_file_click(GtkButton* button, FileClickParams* params) {
    GtkFileChooserNative* file_chooser = gtk_file_chooser_native_new("Open File",
        params->window, GTK_FILE_CHOOSER_ACTION_OPEN, "Open", "Cancel"
    );
    gtk_native_dialog_set_modal(GTK_NATIVE_DIALOG(file_chooser), true);
    gtk_native_dialog_set_transient_for(GTK_NATIVE_DIALOG(file_chooser), params->window);
    g_signal_connect(file_chooser, "response", G_CALLBACK(open_file_response), params->tab_view);
    gtk_native_dialog_show(GTK_NATIVE_DIALOG(file_chooser));
}

static void open_file_response(GtkNativeDialog* dialog, int response, AdwTabView* tab_view) {
    if (response == GTK_RESPONSE_ACCEPT) {
        GFile* file = gtk_file_chooser_get_file(GTK_FILE_CHOOSER(dialog));
        filePath = g_file_get_path(file);
        printf("Open File: %s\n", filePath);
        GtkTextBuffer* buffer = new_tab_page(tab_view);

        char* content;
        gsize length;
        GError* error;
        if (!g_file_load_contents(file, NULL, &content, &length, NULL, &error)) {
            printf("Error opening file \"%s\": %s\n", filePath, error->message);
            free(error);
            exit(0);
        }

        gtk_text_buffer_set_text(buffer, (const char*)content, -1);
        g_object_unref(file);
        free(content);
    }

    g_object_unref(dialog);
}

static void save_file_click(GtkButton* button, FileClickParams* params) {
    GtkTextBuffer* buffer = get_active_page_buffer(params->tab_view);
    
	// If in existing file, just overwrite file with content on TextView
	// (No need for save dialog pop up)	
	if (filePath != NULL) {
		GFile* currFile = g_file_new_for_path((const char*) filePath);		
		GtkTextIter start, end;
		gtk_text_buffer_get_bounds(buffer, &start, &end);
		char* content = gtk_text_buffer_get_text(buffer, &start, 
												&end, false);
		GError* error = NULL;
		gsize length = gtk_text_buffer_get_char_count(buffer);
		
		if(g_file_replace_contents(currFile, content, length, 
		   NULL, false, G_FILE_CREATE_NONE, NULL, NULL, &error) == false){
			printf("%s", error->message);
			g_object_unref(currFile);
			g_object_unref(error);
			exit(0);
		}
		g_object_unref(currFile);
	}
	// We're in a completely new file.
	else {
		GtkFileChooserNative* file_chooser = gtk_file_chooser_native_new("Save File", params->window, GTK_FILE_CHOOSER_ACTION_SAVE, 
																	 	"Save", "Cancel");
		GtkFileChooser* chooser = GTK_FILE_CHOOSER(file_chooser);	
	
		gtk_file_chooser_set_current_name(chooser, "Untitled document.txt");
    	
		gtk_native_dialog_set_modal(GTK_NATIVE_DIALOG(file_chooser), true);
   	    gtk_native_dialog_set_transient_for(GTK_NATIVE_DIALOG(file_chooser), params->window);
    	g_signal_connect(file_chooser, "response", G_CALLBACK(save_file_response), buffer);
    	gtk_native_dialog_show(GTK_NATIVE_DIALOG(file_chooser));
	}
}

static void save_file_response(GtkNativeDialog* dialog, int response, GtkTextBuffer* buffer) {
    if(response == GTK_RESPONSE_ACCEPT){
        // Creating a GFile from file name set in file chooser for dialog
		GFile* file = gtk_file_chooser_get_file(GTK_FILE_CHOOSER(dialog));
		// Saving the current file name
		filePath = g_file_get_path(file);
		
		// Fetching the current content in the file
		GtkTextIter start, end;
		gtk_text_buffer_get_bounds(buffer, &start, &end);
		char* content = gtk_text_buffer_get_text(buffer, &start, &end, false);
		gsize length = gtk_text_buffer_get_char_count(buffer);
		GError* error = NULL;
		
		// Overwriting the file content with the written content in the application
		// Error message will pop up if replacing content fails
		if(g_file_replace_contents(file, content, length, 
		   NULL, false, G_FILE_CREATE_NONE, NULL, NULL, &error) == false){
			printf("%s", error->message);
			g_object_unref(error);
			g_object_unref(file);
			exit(0);
		}
		
		// Free up stuff not needed afterward.
		g_object_unref(file);
	}
	g_object_unref(dialog);
}

static GtkTextBuffer* new_tab_page(AdwTabView* tab_view) {
    Widget scroller = gtk_scrolled_window_new(),
        text_view = gtk_text_view_new();
    
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroller), text_view);
    gtk_widget_set_vexpand(scroller, true);
    gtk_widget_set_size_request(scroller, 400, 200);

    adw_tab_view_append(tab_view, scroller);
    return gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
}
static GtkTextBuffer* get_active_page_buffer(AdwTabView* tab_view) {
    return gtk_text_view_get_buffer(GTK_TEXT_VIEW(
        gtk_scrolled_window_get_child(GTK_SCROLLED_WINDOW(
            adw_tab_page_get_child(
                adw_tab_view_get_selected_page(tab_view)
            )
        ))
    ));
}

void main_window(GtkApplication *app) {
    Widget window,
        window_box,
            headerbar,
                file_new,
                file_open,
                file_save,
                // folder_open,
                share_toggle,
            tabbar,
            tab_view,
                // scroller,
                //     text_view,
            action_bar;
    FileClickParams* file_click_params = malloc(sizeof(FileClickParams));
    MainMalloced* malloced = malloc(sizeof(MainMalloced));
    malloced->file_click_params = file_click_params;
	
    window = adw_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Window");
    g_signal_connect(window, "destroy", G_CALLBACK(main_window_destroy), malloced);

    tab_view = GTK_WIDGET(adw_tab_view_new());
    tabbar = GTK_WIDGET(adw_tab_bar_new());
    adw_tab_bar_set_view(ADW_TAB_BAR(tabbar), ADW_TAB_VIEW(tab_view));

    file_click_params->window = GTK_WINDOW(window);
    file_click_params->tab_view = ADW_TAB_VIEW(tab_view);
	
    headerbar = gtk_header_bar_new();

	file_new = gtk_button_new();
	gtk_button_set_child(GTK_BUTTON(file_new), gtk_image_new_from_icon_name("document-new-symbolic"));
	gtk_widget_set_tooltip_text(file_new, "New File");
	g_signal_connect(file_new, "clicked", G_CALLBACK(new_file_click), file_click_params);
	gtk_header_bar_pack_start(GTK_HEADER_BAR(headerbar), file_new);

    file_open = gtk_button_new();
    gtk_button_set_child(GTK_BUTTON(file_open), gtk_image_new_from_icon_name("text-x-generic-symbolic"));
    gtk_widget_set_tooltip_text(file_open, "Open File");
	g_signal_connect(file_open, "clicked", G_CALLBACK(open_file_click), file_click_params);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(headerbar), file_open);
    
	file_save = gtk_button_new();
	gtk_button_set_child(GTK_BUTTON(file_save), gtk_image_new_from_icon_name("document-save-symbolic"));
    gtk_widget_set_tooltip_text(file_save, "Save File");
	g_signal_connect(file_save, "clicked", G_CALLBACK(save_file_click), file_click_params);
	gtk_header_bar_pack_start(GTK_HEADER_BAR(headerbar), file_save);

	// folder_open = gtk_button_new();
    // gtk_button_set_child(GTK_BUTTON(folder_open), gtk_image_new_from_icon_name("folder-symbolic"));
    // g_signal_connect(folder_open, "clicked", G_CALLBACK(open_folder_click), window);
    // gtk_header_bar_pack_start(GTK_HEADER_BAR(headerbar), folder_open);
    share_toggle = gtk_toggle_button_new_with_label(SERVER_TOGGLE_OFF_TITLE);
    g_signal_connect(share_toggle, "toggled", G_CALLBACK(share_toggle_click), window);
    gtk_header_bar_pack_end(GTK_HEADER_BAR(headerbar), share_toggle);

    action_bar = gtk_action_bar_new();
    gtk_widget_set_vexpand(action_bar, false);
    gtk_action_bar_pack_start(GTK_ACTION_BAR(action_bar), gtk_label_new("Share: Not connected"));

    window_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_append(GTK_BOX(window_box), headerbar);
    gtk_box_append(GTK_BOX(window_box), tab_view);
    gtk_box_append(GTK_BOX(window_box), action_bar);
    adw_application_window_set_content(ADW_APPLICATION_WINDOW(window), window_box);

    gtk_window_present(GTK_WINDOW(window));
}

void main_window_destroy(GtkApplicationWindow* window, MainMalloced* params) {
    free(params->file_click_params);
}

/// Creates the UI for the dialog that allows user to activate sharing.
ShareDialogEntries share_dialog(GtkBox* dialog_content_area) {
    Widget
        host_port,
        connect_box,
            connect_ip,
            connect_port;

    host_port = gtk_entry_new();
    gtk_widget_set_name(host_port, "host-port");
    gtk_entry_set_placeholder_text(GTK_ENTRY(host_port), "Port");
    connect_ip = gtk_entry_new();
    gtk_widget_set_name(connect_ip, "connect-ip");
    gtk_entry_set_placeholder_text(GTK_ENTRY(connect_ip), "IP Address");
    connect_port = gtk_entry_new();
    gtk_widget_set_name(connect_port, "connect-port");
    gtk_entry_set_placeholder_text(GTK_ENTRY(connect_port), "Port");

    connect_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);
    gtk_box_append(GTK_BOX(connect_box), connect_ip);
    gtk_box_append(GTK_BOX(connect_box), gtk_label_new(":"));
    gtk_box_append(GTK_BOX(connect_box), connect_port);

    gtk_box_append(dialog_content_area, gtk_label_new("Host session"));
    gtk_box_append(dialog_content_area, host_port);
    gtk_box_append(dialog_content_area, gtk_label_new("Or:"));
    gtk_box_append(dialog_content_area, gtk_label_new("Connect to an exisitng session"));
    gtk_box_append(dialog_content_area, connect_box);

    ShareDialogEntries r = {
        GTK_ENTRY(host_port),
        GTK_ENTRY(connect_ip),
        GTK_ENTRY(connect_port),
    };
    return r;
}
