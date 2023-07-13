#ifndef __CLIENT_H__
#define __CLIENT_H__

#include "server.h"

StartStatus start_client(const char* ip_address, int port);

void stop_client();

#endif // __CLIENT_H__
