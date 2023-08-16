#ifndef __CLIENT_H__
#define __CLIENT_H__

#include "util.h"

extern bool client_running;

StartStatus start_client(const char* ip_address, int port);

void stop_client();

void client_change_tab_content(TabContent tab_content);

#endif // __CLIENT_H__
