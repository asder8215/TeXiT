#ifndef __SERVER_H__
#define __SERVER_H__

#include "util.h"
//#include "tab-page.h"

#define MAX_CONNECTIONS 3

extern bool server_running;

/// Tries to start a server using the provided port number.
/// Port number must be between 1024 and 65535 (inclusive).
StartStatus start_server(int port);
/// Wonder what this does ?
void stop_server();

/// Send message to clients to append a new tab and its contents.
/// The new tab is always the last tab of **tab_view**.
void server_new_tab();

/// Send message to clients to remove a specific tab page.
void server_remove_tab(AdwTabPage* tab_page);

/// Send content of a tab page to clients.
void server_change_tab_content(TabContent tab_content);

#endif // __SERVER_H__
