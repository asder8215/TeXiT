#ifndef __TAB_PAGE_H__
#define __TAB_PAGE_H__

#include <gtk/gtk.h>
#include <adwaita.h>
#include "buffer.h"

typedef struct {
    AdwTabPage* page;
    EditorBuffer* buffer;
} Page;

/// Append a new Tab Page to **tab_view** with a scorller and a textview using the `EditorBuffer`.
Page new_tab_page(AdwTabView* tab_view, const char* title, const char* filePath);

EditorBuffer* page_get_buffer(AdwTabPage* page); 
Page get_active_page(AdwTabView* tab_view);
Page get_nth_page(AdwTabView* tab_view, size_t i);

gboolean close_tab_page(AdwTabView* tab_view, AdwTabPage* page, GtkWindow* window);

/// Like `Page`, but specifically for the client mode since it shouldn't use the functionality of `EditorBuffer`.
typedef struct {
    AdwTabPage* page;
    GtkTextBuffer* buffer;
} ClientPage;

GtkTextBuffer* client_page_get_buffer(AdwTabPage* page);
/// Like `new_tab_page`, but uses a normal `GtkTextBuffer`.
ClientPage new_client_tab(AdwTabView* tab_view, const char* title);
ClientPage get_nth_client_tab(AdwTabView* tab_view, size_t i);

#endif

