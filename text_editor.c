#include <gtk/gtk.h>
#include <stdio.h>
#include <sys/wait.h>
/**
static void print_hello(GtkWidget *widget, gpointer data){
	//g_print ("Hello World\n");
	printf("Hello World\n");
}
**/

typedef GtkWidget* Widget;
typedef GtkTextBuffer* Buffer;
typedef GtkTextIter* Iter;
typedef GtkFileChooserAction FileAction;

static void filePick(GtkDialog* dialog, int response){
	if(response == GTK_RESPONSE_ACCEPT){
		GtkFileChooser* chooser= GTK_FILE_CHOOSER(dialog);
		GFile* file = gtk_file_chooser_get_file(chooser);
		//open_file(file);
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

	//gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(window));
	gtk_window_present(GTK_WINDOW(dialog));
	g_signal_connect(dialog, "response", G_CALLBACK(filePick), NULL);
}


static void activate(GtkApplication *app, gpointer user_data){
	// Declaring all variables
	Widget window, headerbar, file_btn, server_toggle, view;
	Buffer buffer;
	//Iter itr, start_itr, end_itr;

	
	// Creating the window
	window = gtk_application_window_new(app);
	gtk_window_set_title(GTK_WINDOW(window), "GTK Editor");
 	gtk_window_set_default_size(GTK_WINDOW(window), 300, 300);
	
	// Text View & Buffer
	view = gtk_text_view_new();
	gtk_text_view_set_top_margin(GTK_TEXT_VIEW(view), 3);
	gtk_text_view_set_left_margin(GTK_TEXT_VIEW(view), 3);

	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));

  	//button = gtk_button_new_with_label("Hello World");
  	//g_signal_connect(button, "clicked", G_CALLBACK(print_hello), NULL);
  	//gtk_window_set_child(GTK_WINDOW(window), button);
  
	// Creating the headerbar and associating it with the window
  	headerbar = gtk_header_bar_new();
  	gtk_window_set_titlebar(GTK_WINDOW(window), headerbar);
	
	// File button & Dialog action 
  	file_btn = gtk_button_new_with_label("");
	//dialog = 
  	gtk_button_set_child(GTK_BUTTON(file_btn), gtk_image_new_from_icon_name("folder")); 
  	gtk_header_bar_pack_start(GTK_HEADER_BAR(headerbar), file_btn); 
	g_signal_connect(file_btn, "clicked", G_CALLBACK(fileDialog), view); 	
		
	// Server sharing button
	server_toggle = gtk_toggle_button_new_with_label(""); 
  	gtk_button_set_child(GTK_BUTTON(server_toggle), 
			     gtk_image_new_from_icon_name("mail-replied")); 	      
	gtk_header_bar_pack_start(GTK_HEADER_BAR(headerbar), server_toggle); 
	
	
	//gtk_text_buffer_set_text(buffer, "Hello, this is some text.", -1); // default text
	
	// Iters created for debugging purposes
	//start_itr = gtk_text_buffer_get_start_iter(buffer, itr);
	//end_itr = gtk_text_buffer_get_end_iter(buffer, itr);
	//char* msg = gtk_text_buffer_get_text(buffer, start_itr, end_itr, false);
	//printf(msg);
	
	// Setting the Text View as a child of window
	gtk_window_set_child(GTK_WINDOW(window), view);
  	
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

