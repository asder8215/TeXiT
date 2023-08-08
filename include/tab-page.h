#ifndef __TAB_PAGE_H__
#define __TAB_PAGE_H__

#include <gtk/gtk.h>
#include <adwaita.h>
#include "buffer.h"

typedef struct {
    AdwTabPage* page;
    EditorBuffer* buffer;
} Page;

Page new_tab_page(AdwTabView* tab_view, const char* title, const char* filePath);

Page get_active_page(AdwTabView* tab_view);
Page get_nth_page(AdwTabView* tab_view, size_t i);

gboolean close_tab_page(AdwTabView* tab_view, AdwTabPage* page, GtkWindow* window);

#endif

