#ifndef __CLIENT_H__
#define __CLIENT_H__

#include "util.h"

StartStatus start_client(const char* ip_address, int port, AdwTabView* tab_view);

void stop_client();

#endif // __CLIENT_H__
