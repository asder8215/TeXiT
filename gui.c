#include "gui.h"
#include "tab-page.h"
#include <stdio.h>
#include <string.h>
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

static const char* SERVER_TOGGLE_ON_TITLE = "✔️ Share";
static const char* SERVER_TOGGLE_OFF_TITLE = "❌ Share";
static const char* SHARE_RESPONSE_CANCEL = "cancel";
static const char* SHARE_RESPONSE_HOST = "host";
static const char* SHARE_RESPONSE_CONNECT = "connect";
// Minimum dimensions of the Window's main content (the TextView area).
static const unsigned int CONTENT_MIN_WIDTH = 400;
static const unsigned int CONTENT_MIN_HEIGHT = 200;

/// Handler for activating/deactivating the share feature. A GtkDialog will be crated on top of *window*.
static void share_toggle_click(GtkToggleButton* toggle, GtkWindow* window) {
    if (gtk_toggle_button_get_active(toggle)) {
        // Deactivate toggle button. `share_enable_response()` should activate the toggle button if setup was successful.
        gtk_toggle_button_set_active(toggle, false);

        // ~~Freed by `share_enable_response()`.~~
        // Gives error when `g_free(builder), even though it should be freed accroding to https://docs.gtk.org/gtk4/ctor.Builder.new_from_resource.html
        GtkBuilder* builder = gtk_builder_new_from_resource("/me/Asder8215/TextEditor/share-dialog.ui");
        AdwMessageDialog* dialog = ADW_MESSAGE_DIALOG(gtk_builder_get_object(builder, "dialog"));
        ShareDialogEntries entries = share_dialog_entries(builder);
        // C moment :( Why must it be done like this
        // Freed by `share_enable_response()`.
        ShareEnableParams* params = malloc(sizeof(ShareEnableParams));
        params->toggle = GTK_BUTTON(toggle);
        params->entries = entries;

        // Connect response callback
        gtk_window_set_transient_for(GTK_WINDOW(dialog), window);
        g_signal_connect(dialog, "response", G_CALLBACK(share_enable_response), params);
        gtk_window_present(GTK_WINDOW(dialog));
    } else {
        gtk_button_set_label(GTK_BUTTON(toggle), SERVER_TOGGLE_OFF_TITLE);
    }
}

/// Set up sharing connection (either hosting or connecting).
static void share_enable_response(AdwMessageDialog* dialog, const char* response, ShareEnableParams* params) {
    // The response_id ptr will be different than that of any of the set response_ids in `share_toggle_click()`, dont know why. Must use `strcmp()`.
    // strcmp() == 0 when strings are equal.
    if (strcmp(response, SHARE_RESPONSE_HOST) == 0) {
        const char* port = gtk_editable_get_text(params->entries.host_port);

        printf("Host with port %s\n", port);
    } else if (strcmp(response, SHARE_RESPONSE_CONNECT) == 0) {
        const char* ip = gtk_editable_get_text(params->entries.connect_ip);
        const char* port = gtk_editable_get_text(params->entries.connect_port);

        printf("Connect to %s with port %s\n", ip, port);
    } else {
        return;
    }
    // TODO: only change label if server start was successful.
    gtk_button_set_label(params->toggle, SERVER_TOGGLE_ON_TITLE);
    free(params);
}


static void new_file_click(GtkButton* button, FileClickParams* params) {
    /*
    if(gtk_widget_get_visible(GTK_WIDGET(params->label))){
        gtk_widget_set_visible(GTK_WIDGET(params->label), false);
    }
    */

    new_tab_page(params->tab_view, "Untitled", NULL);

    /*
    if(adw_tab_view_get_n_pages(params->tab_view) == 1){
        gtk_widget_set_visible(GTK_WIDGET(params->tabbar), true);
    }
    */
}


static void open_file_click(GtkButton* button, FileClickParams* params) {
    /*
    if(gtk_widget_get_visible(GTK_WIDGET(params->label))){
        gtk_widget_set_visible(GTK_WIDGET(params->label), false);
    }
    */
    GtkFileChooserNative* file_chooser = gtk_file_chooser_native_new("Open File",
        params->window, GTK_FILE_CHOOSER_ACTION_OPEN, "Open", "Cancel"
    );
    gtk_native_dialog_set_modal(GTK_NATIVE_DIALOG(file_chooser), true);
    gtk_native_dialog_set_transient_for(GTK_NATIVE_DIALOG(file_chooser), params->window);
    g_signal_connect(file_chooser, "response", G_CALLBACK(open_file_response), params);
    gtk_native_dialog_show(GTK_NATIVE_DIALOG(file_chooser));
}

static void open_file_response(GtkNativeDialog* dialog, int response, FileClickParams* params) {
    if (response == GTK_RESPONSE_ACCEPT) {
        GFile* file = gtk_file_chooser_get_file(GTK_FILE_CHOOSER(dialog));
        const char* filePath = g_file_get_path(file);
        printf("Open File: %s\n", filePath);
        GtkTextBuffer* buffer = new_tab_page(params->tab_view, g_file_get_basename(file), filePath).buffer;

        /*
        if(adw_tab_view_get_n_pages(params->tab_view) == 1){
            gtk_widget_set_visible(GTK_WIDGET(params->tabbar), true);
        }
        */

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

void save_file_click(GtkButton* button, FileClickParams* params) {
    // could be NULL if no tabs are open
    Page page = get_active_page(params->tab_view);
    if (page.page == NULL) {
        adw_toast_overlay_add_toast(params->toast_overlay, adw_toast_new("No documents are open"));
        return;
    }
   
	// Grabbing filePath from tab page. 
	const char* filePath = (const char*) g_object_get_data(G_OBJECT(page.page),
	   													   "file_path");

	//const char* filePath = adw_tab_page_get_keyword(page.page);
	// If in existing file, just overwrite file with content on TextView
	// (No need for save dialog pop up)	
	if(filePath != NULL) {
		GFile* currFile = g_file_new_for_path((const char*) filePath);		
		GtkTextIter start, end;
		gtk_text_buffer_get_bounds(page.buffer, &start, &end);
		char* content = gtk_text_buffer_get_text(page.buffer, &start, &end, false);
		GError* error = NULL;
		//gsize length = gtk_text_buffer_get_char_count(page.buffer);
		gsize length = strlen(content);

		if(!g_file_replace_contents(currFile, content, length, 
		   NULL, false, G_FILE_CREATE_NONE, NULL, NULL, &error)){
			printf("%s", error->message);
			g_object_unref(currFile);
			g_object_unref(error);
			exit(0);
		}
		g_object_unref(currFile);
        
        // small hack to free up params from the tab page close event.
        if(params->toast_overlay == NULL){
            adw_tab_view_close_page_finish(params->tab_view, page.page, true);
            free(params);
        }
	}
	// We're in a completely new file.
	else {
		GtkFileChooserNative* file_chooser = gtk_file_chooser_native_new("Save File", params->window, GTK_FILE_CHOOSER_ACTION_SAVE, 
																	 	"Save", "Cancel");
		GtkFileChooser* chooser = GTK_FILE_CHOOSER(file_chooser);	
	
		gtk_file_chooser_set_current_name(chooser, "Untitled document.txt");
    	
		gtk_native_dialog_set_modal(GTK_NATIVE_DIALOG(file_chooser), true);
   	    gtk_native_dialog_set_transient_for(GTK_NATIVE_DIALOG(file_chooser), params->window);
    	g_signal_connect(file_chooser, "response", G_CALLBACK(save_file_response), params);
    	gtk_native_dialog_show(GTK_NATIVE_DIALOG(file_chooser));
	}
}

static void save_file_response(GtkNativeDialog* dialog, int response, FileClickParams* params) {
    // page or buffer are Not NULL, this is only called if there was a tab to save.
    Page page = get_active_page(params->tab_view);

    if(response == GTK_RESPONSE_ACCEPT){
        // Creating a GFile from file name set in file chooser for dialog
		GFile* file = gtk_file_chooser_get_file(GTK_FILE_CHOOSER(dialog));
		// Saving the current file name
		const char* filePath = g_file_get_path(file);
		g_object_set_data_full(G_OBJECT(page.page), "file_path", g_strdup(filePath), (GDestroyNotify) g_free);
		
        // Set tab title
        adw_tab_page_set_title(page.page, g_file_get_basename(file));
		
		// Fetching the current content in the file
		GtkTextIter start, end;
		gtk_text_buffer_get_bounds(page.buffer, &start, &end);
		char* content = gtk_text_buffer_get_text(page.buffer, &start, &end, false);
		//gsize length = gtk_text_buffer_get_char_count(page.buffer);
		gsize length = strlen(content);
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

    // small hack to free up params from the tab page close event.
    if(params->toast_overlay == NULL){
        adw_tab_view_close_page_finish(params->tab_view, page.page, true);
        free(params);
    }
}

void main_window(GtkApplication *app) {
    GtkBuilder* builder = gtk_builder_new_from_resource("/me/Asder8215/TextEditor/main-window.ui");

    FileClickParams* file_click_params = malloc(sizeof(FileClickParams));
    MainMalloced* malloced = malloc(sizeof(MainMalloced));
    malloced->file_click_params = file_click_params;

    GtkWindow* window = GTK_WINDOW(gtk_builder_get_object(builder, "main-window"));
    gtk_window_set_application(window, app);
    g_signal_connect(window, "destroy", G_CALLBACK(main_window_destroy), malloced);

    file_click_params->window = window;
    file_click_params->tab_view = ADW_TAB_VIEW(gtk_builder_get_object(builder, "tab-view"));
    file_click_params->toast_overlay = ADW_TOAST_OVERLAY(gtk_builder_get_object(builder, "toast-overlay"));

    g_signal_connect(file_click_params->tab_view, "close-page", G_CALLBACK(close_tab_page), GTK_WINDOW(window));

    GtkButton* file_new = GTK_BUTTON(gtk_builder_get_object(builder, "file-new"));
    g_signal_connect(file_new, "clicked", G_CALLBACK(new_file_click), file_click_params);
    GtkButton* file_open = GTK_BUTTON(gtk_builder_get_object(builder, "file-open"));
    g_signal_connect(file_open, "clicked", G_CALLBACK(open_file_click), file_click_params);
    GtkButton* file_save = GTK_BUTTON(gtk_builder_get_object(builder, "file-save"));
    g_signal_connect(file_save, "clicked", G_CALLBACK(save_file_click), file_click_params);

    GtkToggleButton* share_toggle = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "share-toggle"));
    gtk_button_set_label(GTK_BUTTON(share_toggle), SERVER_TOGGLE_OFF_TITLE);
    g_signal_connect(share_toggle, "toggled", G_CALLBACK(share_toggle_click), window);

    gtk_window_present(window);
}

void main_window_destroy(GtkApplicationWindow* window, MainMalloced* params) {
    free(params->file_click_params);
}


/// Callback for when user presses Enter on the entry.
/// Emtits the `response` signal with response_id "host".
static void host_port_activate(GtkEditable* entry, AdwMessageDialog* dialog) {
        adw_message_dialog_response(dialog,SHARE_RESPONSE_HOST);
}
/// Callback for when user presses Enter on the entry.
/// Shifts focus to the `connect_port` entry.
static void connect_ip_activate(GtkEditable* entry, GtkEditable* connect_port) {
    gtk_widget_grab_focus(GTK_WIDGET(connect_port));
}
/// Callback for when user presses Enter on the entry.
/// Emtits the `response` signal with response_id "connect".
static void connect_port_activate(GtkEditable* entry, AdwMessageDialog* dialog) {
    adw_message_dialog_response(dialog,SHARE_RESPONSE_CONNECT);
}
ShareDialogEntries share_dialog_entries(GtkBuilder* dialog_builder) {
    AdwMessageDialog* dialog = ADW_MESSAGE_DIALOG(gtk_builder_get_object(dialog_builder, "dialog"));
    GtkEditable* host_port = GTK_EDITABLE(gtk_builder_get_object(dialog_builder, "host-port"));
    GtkEditable* connect_ip = GTK_EDITABLE(gtk_builder_get_object(dialog_builder, "connect-ip"));
    GtkEditable* connect_port = GTK_EDITABLE(gtk_builder_get_object(dialog_builder, "connect-port"));
    // Connect Entry callbacks for when user presses Enter
    g_signal_connect(host_port, "entry-activated", G_CALLBACK(host_port_activate), dialog);
    g_signal_connect(connect_ip, "entry-activated", G_CALLBACK(connect_ip_activate), connect_port);
    g_signal_connect(connect_port, "entry-activated", G_CALLBACK(connect_port_activate), dialog);

    ShareDialogEntries r = {
        GTK_EDITABLE(host_port),
        GTK_EDITABLE(connect_ip),
        GTK_EDITABLE(connect_port),
    };
    return r;
}
