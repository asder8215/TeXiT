#include "gui.h"
#include "tab-page.h"
#include "server.h"
#include <stdio.h>

Page new_tab_page(AdwTabView* tab_view, const char* title, const char* filePath) {
    Widget scroller, text_view;
    Page rtrn;
    
    scroller = gtk_scrolled_window_new();
    rtrn.page = adw_tab_view_append(tab_view, scroller);
    rtrn.buffer = editor_buffer_new(filePath, rtrn.page);
    text_view = gtk_text_view_new_with_buffer(GTK_TEXT_BUFFER(rtrn.buffer));
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroller), text_view);

    adw_tab_page_set_title(rtrn.page, title);
    adw_tab_page_set_icon(rtrn.page, g_themed_icon_new("text-x-generic-symbolic"));
    
    // set visibility of the tab view to on upon first new tab.
    if(!gtk_widget_get_visible(GTK_WIDGET(tab_view))){
        gtk_widget_set_visible(GTK_WIDGET(tab_view), true);
    }
    
	return rtrn;
}

EditorBuffer* page_get_buffer(AdwTabPage* page) {
    if (page == NULL)
        return NULL;
    else
        return EDITOR_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(
            gtk_scrolled_window_get_child(GTK_SCROLLED_WINDOW(
                adw_tab_page_get_child(page)
            ))
        )));
}
Page get_active_page(AdwTabView* tab_view) {
    Page page;
    page.page = adw_tab_view_get_selected_page(tab_view);
    page.buffer = page_get_buffer(page.page);
	return page;
}
Page get_nth_page(AdwTabView* tab_view, size_t n) {
    Page page;
    page.page = adw_tab_view_get_nth_page(tab_view, n);
    page.buffer = page_get_buffer(page.page);
    return page;
}

typedef struct {
    GtkWindow* window;
    AdwTabView* tab_view;
    AdwTabPage* target_page;
} CloseUnsavedParams;

/// Handles response receive from the close tab page message dialog.
static void close_unsaved_tab_response(AdwMessageDialog* dialog, GAsyncResult* result, CloseUnsavedParams* params) {  
    const char* response = adw_message_dialog_choose_finish(dialog, result);
    EditorBuffer* buffer = page_get_buffer(params->target_page);

    // save response
    if (strcmp(response, "save") == 0) {
        // calls tab_view.close_page_finish(true) if file was successfully saved
        server_remove_tab(params->tab_view, params->target_page);
        editor_buffer_save(buffer, params->tab_view, params->window, true);
    }
    // close response
    else if (strcmp(response, "close") == 0) { 
        server_remove_tab(params->tab_view, params->target_page);
        adw_tab_view_close_page_finish(params->tab_view, params->target_page, true);
    }
    // cancel response
    else if (strcmp(response, "cancel") == 0)
        adw_tab_view_close_page_finish(params->tab_view, params->target_page, false);

    if (adw_tab_view_get_n_pages(params->tab_view) == 0)
        gtk_widget_set_visible(GTK_WIDGET(params->tab_view), false);

    free(params);
}

/// Handler for closing a tab page.
gboolean close_tab_page(AdwTabView* tab_view, AdwTabPage* page, GtkWindow* window) {
    EditorBuffer* buffer = page_get_buffer(page);
    const char* file_path = editor_buffer_get_file_path(buffer);
    
    GFile* file;
    if(file_path != NULL)
        file = g_file_new_for_path(file_path);
    else
        file = NULL;

    const char* file_name;
    // file_name utilized for the close tab message dialog
    if(file != NULL)
        file_name = g_file_get_basename(file);
    else
        file_name = "Untitled";
    
    // Prompt user with if they want to cancel closing the tab, closing the
    // tab even with unsaved changes, or save the content in the tab.
    if(editor_buffer_get_edited(buffer)){
        AdwMessageDialog* dialog = ADW_MESSAGE_DIALOG(adw_message_dialog_new(GTK_WINDOW(window), "Save File?", NULL));

        adw_message_dialog_format_body(ADW_MESSAGE_DIALOG(dialog), "There are unsaved changes in %s.",
                                       file_name);

        adw_message_dialog_add_responses(ADW_MESSAGE_DIALOG(dialog), "cancel", "Cancel", "close", "Close", 
                                         "save", "Save", NULL);

        adw_message_dialog_set_response_appearance(ADW_MESSAGE_DIALOG(dialog), "save",
                                                   ADW_RESPONSE_SUGGESTED);

        adw_message_dialog_set_response_appearance(ADW_MESSAGE_DIALOG(dialog), "close",
                                                   ADW_RESPONSE_DESTRUCTIVE);
        
        
        adw_message_dialog_set_default_response(ADW_MESSAGE_DIALOG(dialog), "cancel");
        adw_message_dialog_set_close_response(ADW_MESSAGE_DIALOG(dialog), "cancel");

        CloseUnsavedParams* params = malloc(sizeof(CloseUnsavedParams));
        params->window = GTK_WINDOW(dialog);
        params->tab_view = tab_view;
        params->target_page = page;

        gtk_window_set_modal(GTK_WINDOW(dialog), true);
        gtk_window_set_transient_for(GTK_WINDOW(dialog), window);
        gtk_window_set_resizable(GTK_WINDOW(dialog), false);
        
        // Instead of using g_signal_connect and gtk_window_present
        // This method allows for an async call on the desired function.
        adw_message_dialog_choose(ADW_MESSAGE_DIALOG(dialog), NULL, (GAsyncReadyCallback) close_unsaved_tab_response, params);
    }
    // Close the tab if nothing's changed.
    else{
        server_remove_tab(tab_view, page);
        adw_tab_view_close_page_finish(tab_view, page, true);
        if (adw_tab_view_get_n_pages(tab_view) == 0)
            gtk_widget_set_visible(GTK_WIDGET(tab_view), false);
    }

    // This return value prevents default handlers from being called on
    // ^Took me forever to figure this out.
    return GDK_EVENT_STOP;
}

