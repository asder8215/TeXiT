#include <gtk/gtk.h>
#include <stdio.h>
#include <sys/wait.h>

typedef GtkWidget* Widget;
typedef GtkTextBuffer* Buffer;
typedef GtkTextIter* Iter;
typedef GtkFileChooserAction FileAction;

static void filePick(GtkDialog* dialog, int response){
	// Fetch the parent window that the File dialog is connected to
	GtkWindow* window = gtk_window_get_transient_for(GTK_WINDOW(dialog));
	GtkTextBuffer* buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(
				gtk_window_get_child(window)));
	// GList* children = NULL;
	// GtkWidget* view = NULL;
	/*
	if(GTK_IS_CONTAINER(window)){
		children = gtk_container_get_children(GTK_CONTAINER(window));
	}
	*/
	if(response == GTK_RESPONSE_ACCEPT){
		// Choosing a file and loading contents into Window's TextView
		GtkFileChooser* chooser= GTK_FILE_CHOOSER(dialog);
		GFile* file = gtk_file_chooser_get_file(chooser);
		char* filePath = g_file_get_path(file);
		char* contents;
		gsize length;
		
		g_file_load_contents(file, NULL, &contents, 
				     &length, NULL, NULL);
		//g_print("%s", contents);
		gtk_text_buffer_set_text(buffer, (const char*) contents, 
				         length);
	}
	gtk_window_destroy(GTK_WINDOW(dialog));
}

static void fileDialog(GtkWidget* file_btn, gpointer window){
	Widget dialog;
	dialog = gtk_file_chooser_dialog_new("Choose a file",
					    GTK_WINDOW(window),
					    GTK_FILE_CHOOSER_ACTION_OPEN,
					    "Cancel",
					    GTK_RESPONSE_CANCEL,
					    "Open",
					    GTK_RESPONSE_ACCEPT,
					    NULL);
	gtk_window_set_modal(GTK_WINDOW(dialog), true);
	gtk_window_set_transient_for(GTK_WINDOW(dialog),
					    GTK_WINDOW(window));
	
	gtk_window_present(GTK_WINDOW(dialog));
	g_signal_connect(dialog, "response", G_CALLBACK(filePick), NULL);
}


static void activate(GtkApplication *app, gpointer user_data){
	// Declaring all variables
	Widget window, headerbar, file_btn, server_toggle, view;
	Buffer buffer;
	
	// Creating the window
	window = gtk_application_window_new(app);
	gtk_window_set_title(GTK_WINDOW(window), "GTK Editor");
 	gtk_window_set_default_size(GTK_WINDOW(window), 300, 300);
	
	// Text View & Buffer (Styling and Name Included)
	view = gtk_text_view_new();
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
	
	//gtk_widget_set_name(view, "Editor TextView");
	gtk_text_view_set_top_margin(GTK_TEXT_VIEW(view), 3);
	gtk_text_view_set_left_margin(GTK_TEXT_VIEW(view), 3);
	gtk_window_set_child(GTK_WINDOW(window), view);

	// Creating the headerbar and associating it with the window
  	headerbar = gtk_header_bar_new();
  	gtk_window_set_titlebar(GTK_WINDOW(window), headerbar);
	
	// File button & FileChooser action 
  	file_btn = gtk_button_new_with_label("");
  	gtk_button_set_child(GTK_BUTTON(file_btn), 
			     gtk_image_new_from_icon_name("folder")); 
  	gtk_header_bar_pack_start(GTK_HEADER_BAR(headerbar), file_btn); 
	g_signal_connect(file_btn, "clicked", G_CALLBACK(fileDialog), view); 	
		
	// Server sharing button
	server_toggle = gtk_toggle_button_new_with_label(""); 
  	gtk_button_set_child(GTK_BUTTON(server_toggle), 
			     gtk_image_new_from_icon_name("mail-replied")); 
	gtk_header_bar_pack_start(GTK_HEADER_BAR(headerbar), server_toggle); 
	
  	
	gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv){
  	GtkApplication *app;
  	int status;

  	app = gtk_application_new("org.gtk.example", G_APPLICATION_FLAGS_NONE);
  	g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
  	status = g_application_run(G_APPLICATION(app), argc, argv);
  	g_object_unref(app);

  	return status;
}

