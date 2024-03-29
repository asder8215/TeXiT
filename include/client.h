#ifndef __CLIENT_H__
#define __CLIENT_H__

#include "util.h"

StartStatus start_client(const char* ip_address, int port, AdwTabView* tab_view, FileButtons* file_buttons, GtkToggleButton* toggle, GtkLabel* label, GtkWindow* window);

void stop_client();

void client_change_tab_content(TabContent tab_content);

#endif // __CLIENT_H__
