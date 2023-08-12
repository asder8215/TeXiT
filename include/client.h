#ifndef __CLIENT_H__
#define __CLIENT_H__

#include "util.h"

StartStatus start_client(const char* ip_address, int port, AdwTabView* tab_view, FileButtons* file_buttons, GtkLabel* label);

void stop_client();

void client_change_tab_content(const char* content, unsigned int tab_idx);

#endif // __CLIENT_H__
