#ifndef __TAB_PAGE_H__
#define __TAB_PAGE_H__

//#include <gtk/gtk.h>
//#include <adwaita.h>

#include <gtk/gtk.h>
#include <adwaita.h>
#include "gui.h"

typedef struct {
    AdwTabPage* page;
    GtkTextBuffer* buffer;
} Page;

Page new_tab_page(AdwTabView* tab_view, const char* title, const char* filePath);

Page get_active_page(AdwTabView* tab_view);

gboolean close_tab_page(AdwTabView* tab_view, AdwTabPage* page, GtkWindow* window);



#endif


