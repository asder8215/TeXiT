#include "gui.h"
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

// Global variable for file name (will be changed later)
//static char* filePath = NULL;

/// Handler for activating/deactivating the share feature. A GtkDialog will be crated on top of *window*.
static void share_toggle_click(GtkToggleButton* toggle, GtkWindow* window) {
    const char* label;
    if (gtk_toggle_button_get_active(toggle)) {
        // Deactivate toggle button. `share_enable_response()` should activate the toggle button if setup was successful.
        gtk_toggle_button_set_active(toggle, false);

        // Freed by `share_enable_response()`.
        AdwMessageDialog* dialog = ADW_MESSAGE_DIALOG(adw_message_dialog_new(window, "Start Sharing", NULL));
        // Why did Adwaita make the IDs a string instead of an int???
        adw_message_dialog_add_response(dialog, SHARE_RESPONSE_CANCEL, "Cancel");
        // TODO: Check entry is not empty whe user clicks one of these.
        adw_message_dialog_add_response(dialog, SHARE_RESPONSE_HOST, "Host");
        adw_message_dialog_add_response(dialog, SHARE_RESPONSE_CONNECT, "Connect");
        adw_message_dialog_set_response_appearance (ADW_MESSAGE_DIALOG (dialog), "cancel", ADW_RESPONSE_DESTRUCTIVE);
        adw_message_dialog_set_default_response (ADW_MESSAGE_DIALOG (dialog), "cancel");
        adw_message_dialog_set_close_response (ADW_MESSAGE_DIALOG (dialog), "cancel");

        ShareDialogEntries entries = set_share_dialog_child(dialog);
        // C moment :( Why must it be done like this
        // Freed by `share_enable_response()`.
        ShareEnableParams* params = malloc(sizeof(ShareEnableParams));
        params->toggle = GTK_BUTTON(toggle);
        params->entries = entries;

        // Make the dialog a modal
        gtk_window_set_modal(GTK_WINDOW(dialog), true);
        gtk_window_set_transient_for(GTK_WINDOW(dialog), window);
        gtk_window_set_resizable(GTK_WINDOW(dialog), false);
        // Connect response callback
        g_signal_connect(dialog, "response", G_CALLBACK(share_enable_response), params);
        gtk_window_present(GTK_WINDOW(dialog));
    } else {
        label = SERVER_TOGGLE_OFF_TITLE;
        gtk_button_set_label(GTK_BUTTON(toggle), label);
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
    gtk_window_destroy(GTK_WINDOW(dialog));
    free(params);
}


static void new_file_click(GtkButton* button, FileClickParams* params) {
    if(gtk_widget_get_visible(GTK_WIDGET(params->label))){
        gtk_widget_set_visible(GTK_WIDGET(params->label), false);
    }

    new_tab_page(params->tab_view, "Untitled", NULL);

    if(adw_tab_view_get_n_pages(params->tab_view) == 1){
        gtk_widget_set_visible(GTK_WIDGET(params->tabbar), true);
    }
}


static void open_file_click(GtkButton* button, FileClickParams* params) {
    if(gtk_widget_get_visible(GTK_WIDGET(params->label))){
        gtk_widget_set_visible(GTK_WIDGET(params->label), false);
    }
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

        if(adw_tab_view_get_n_pages(params->tab_view) == 1){
            gtk_widget_set_visible(GTK_WIDGET(params->tabbar), true);
        }

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
		char* content = gtk_text_buffer_get_text(page.buffer, &start, 
												&end, false);
		GError* error = NULL;
		gsize length = gtk_text_buffer_get_char_count(page.buffer);
		
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
		gsize length = gtk_text_buffer_get_char_count(page.buffer);
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

static Page new_tab_page(AdwTabView* tab_view, const char* title, const char* filePath) {
    Widget scroller = gtk_scrolled_window_new(),
        text_view = gtk_text_view_new();
    AdwTabPage* tab_page;
    Page rtrn;
    
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroller), text_view);

    tab_page = adw_tab_view_append(tab_view, scroller);
    adw_tab_page_set_title(tab_page, title);
    adw_tab_page_set_icon(tab_page, g_themed_icon_new("text-x-generic-symbolic"));

    rtrn.page = tab_page;
    rtrn.buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
	// stores the file path to the heap for this specific tab page (calls on g_free when
	// the tab page is destroyed).
	g_object_set_data_full(G_OBJECT(rtrn.page), "file_path", g_strdup(filePath), (GDestroyNotify) g_free);
   
	return rtrn;
}
static Page get_active_page(AdwTabView* tab_view) {
    Page rtrn;
    rtrn.page = adw_tab_view_get_selected_page(tab_view);
    if (rtrn.page != NULL)
        rtrn.buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(
            gtk_scrolled_window_get_child(GTK_SCROLLED_WINDOW(
                adw_tab_page_get_child(rtrn.page)
            ))
        ));
    else
        rtrn.buffer = NULL;

	return rtrn;
}

void main_window(GtkApplication *app) {
    Widget window,
        window_box,
            headerbar,
                title,
                file_new,
                file_open,
                file_save,
                // folder_open,
                share_toggle,
            label,
            tabbar,
            toast_overlay,
                tab_view,
                    // scroller,
                    //     text_view,
            action_bar;

    FileClickParams* file_click_params = malloc(sizeof(FileClickParams));
    MainMalloced* malloced = malloc(sizeof(MainMalloced));
    malloced->file_click_params = file_click_params;
	
    window = adw_application_window_new(app);
    title = adw_window_title_new("Text Editor", NULL);
    g_signal_connect(window, "destroy", G_CALLBACK(main_window_destroy), malloced);

    headerbar = adw_header_bar_new();
    adw_header_bar_set_title_widget(ADW_HEADER_BAR(headerbar), title);

	file_new = gtk_button_new();
	gtk_button_set_child(GTK_BUTTON(file_new), gtk_image_new_from_icon_name("document-new-symbolic"));
	gtk_widget_set_tooltip_text(file_new, "New File");
	g_signal_connect(file_new, "clicked", G_CALLBACK(new_file_click), file_click_params);
	adw_header_bar_pack_start(ADW_HEADER_BAR(headerbar), file_new);

    file_open = gtk_button_new();
    gtk_button_set_child(GTK_BUTTON(file_open), gtk_image_new_from_icon_name("text-x-generic-symbolic"));
    gtk_widget_set_tooltip_text(file_open, "Open File");
	g_signal_connect(file_open, "clicked", G_CALLBACK(open_file_click), file_click_params);
    adw_header_bar_pack_start(ADW_HEADER_BAR(headerbar), file_open);
    
	file_save = gtk_button_new();
	gtk_button_set_child(GTK_BUTTON(file_save), gtk_image_new_from_icon_name("document-save-symbolic"));
    gtk_widget_set_tooltip_text(file_save, "Save File");
	g_signal_connect(file_save, "clicked", G_CALLBACK(save_file_click), file_click_params);
	adw_header_bar_pack_start(ADW_HEADER_BAR(headerbar), file_save);
    
    label = gtk_label_new("Create a new file or open a file");
    gtk_widget_set_vexpand(label, true);
    gtk_widget_set_size_request(label, CONTENT_MIN_WIDTH, CONTENT_MIN_HEIGHT);
    gtk_label_set_yalign(GTK_LABEL(label), CONTENT_MIN_HEIGHT/2);
     
    tab_view = GTK_WIDGET(adw_tab_view_new());
    gtk_widget_set_vexpand(tab_view, true);
    gtk_widget_set_size_request(tab_view, CONTENT_MIN_WIDTH, CONTENT_MIN_HEIGHT);
    tabbar = GTK_WIDGET(adw_tab_bar_new());
    adw_tab_view_set_default_icon(ADW_TAB_VIEW(tab_view), g_themed_icon_new("text-x-generic-symbolic"));
    gtk_widget_set_visible(tabbar, false);
    adw_tab_bar_set_view(ADW_TAB_BAR(tabbar), ADW_TAB_VIEW(tab_view));
    
    toast_overlay = adw_toast_overlay_new();
    adw_toast_overlay_set_child(ADW_TOAST_OVERLAY(toast_overlay), tab_view);

    file_click_params->window = GTK_WINDOW(window);
    file_click_params->label = GTK_LABEL(label);
    file_click_params->tabbar = ADW_TAB_BAR(tabbar);
    file_click_params->tab_view = ADW_TAB_VIEW(tab_view);
    file_click_params->toast_overlay = ADW_TOAST_OVERLAY(toast_overlay);

	// folder_open = gtk_button_new();
    // gtk_button_set_child(GTK_BUTTON(folder_open), gtk_image_new_from_icon_name("folder-symbolic"));
    // g_signal_connect(folder_open, "clicked", G_CALLBACK(open_folder_click), window);
    // adw_header_bar_pack_start(ADW_HEADER_BAR(headerbar), folder_open);
    share_toggle = gtk_toggle_button_new_with_label(SERVER_TOGGLE_OFF_TITLE);
    g_signal_connect(share_toggle, "toggled", G_CALLBACK(share_toggle_click), window);
    adw_header_bar_pack_end(ADW_HEADER_BAR(headerbar), share_toggle);

    action_bar = gtk_action_bar_new();
    gtk_widget_set_vexpand(action_bar, false);
    gtk_action_bar_pack_start(GTK_ACTION_BAR(action_bar), gtk_label_new("Share: Not connected"));

    window_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_append(GTK_BOX(window_box), headerbar);
    gtk_box_append(GTK_BOX(window_box), label);
    gtk_box_append(GTK_BOX(window_box), tabbar);
    gtk_box_append(GTK_BOX(window_box), toast_overlay);
    gtk_box_append(GTK_BOX(window_box), action_bar);
    adw_application_window_set_content(ADW_APPLICATION_WINDOW(window), window_box);

    gtk_window_present(GTK_WINDOW(window));
}

void main_window_destroy(GtkApplicationWindow* window, MainMalloced* params) {
    free(params->file_click_params);
}


/// Callback for when user presses Enter on the entry.
/// Emtits the `response` signal with response_id "host".
static void host_port_activate(AdwEntryRow* entry, AdwMessageDialog* dialog) {
    adw_message_dialog_response (dialog,SHARE_RESPONSE_HOST);
}
/// Callback for when user presses Enter on the entry.
/// Shifts focus to the `connect_port` entry.
static void connect_ip_activate(AdwEntryRow* entry, AdwEntryRow* connect_port) {
    gtk_widget_grab_focus(GTK_WIDGET(connect_port));
}
/// Callback for when user presses Enter on the entry.
/// Emtits the `response` signal with response_id "connect".
static void connect_port_activate(AdwEntryRow* entry, AdwMessageDialog* dialog) {
    adw_message_dialog_response (dialog,SHARE_RESPONSE_CONNECT);
}
ShareDialogEntries set_share_dialog_child(AdwMessageDialog* dialog) {
    Widget container,
        host_group,
            host_port,
        connect_group,
            connect_ip,
            connect_port;

    host_port = adw_entry_row_new();
    adw_preferences_row_set_title(ADW_PREFERENCES_ROW(host_port), "Port");
    connect_ip = adw_entry_row_new();
    adw_preferences_row_set_title(ADW_PREFERENCES_ROW(connect_ip), "IP Address");
    connect_port = adw_entry_row_new();
    adw_preferences_row_set_title(ADW_PREFERENCES_ROW(connect_port), "Port");
    // Connect Entry callbacks for when user presses Enter
    g_signal_connect(host_port, "entry-activated", G_CALLBACK(host_port_activate), dialog);
    g_signal_connect(connect_ip, "entry-activated", G_CALLBACK(connect_ip_activate), connect_port);
    g_signal_connect(connect_port, "entry-activated", G_CALLBACK(connect_port_activate), dialog);

    host_group = adw_preferences_group_new();
    adw_preferences_group_set_title(ADW_PREFERENCES_GROUP(host_group), "Host");
    adw_preferences_group_set_description(ADW_PREFERENCES_GROUP(host_group), "Create a Share session for others to connect to");
    adw_preferences_group_add(ADW_PREFERENCES_GROUP(host_group), host_port);
    connect_group = adw_preferences_group_new();
    adw_preferences_group_set_title(ADW_PREFERENCES_GROUP(connect_group), "Connect");
    adw_preferences_group_set_description(ADW_PREFERENCES_GROUP(connect_group), "Connect to someone else's Share session");
    adw_preferences_group_add(ADW_PREFERENCES_GROUP(connect_group), connect_ip);
    adw_preferences_group_add(ADW_PREFERENCES_GROUP(connect_group), connect_port);

    container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_box_append(GTK_BOX(container), host_group);
    gtk_box_append(GTK_BOX(container), connect_group);

    // Set dialog content
    adw_message_dialog_set_extra_child(dialog, container);

    ShareDialogEntries r = {
        GTK_EDITABLE(host_port),
        GTK_EDITABLE(connect_ip),
        GTK_EDITABLE(connect_port),
    };
    return r;
}
