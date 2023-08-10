#ifndef __SERVER_H__
#define __SERVER_H__

#include "util.h"

#define MAX_CONNECTIONS 3

/// Tries to start a server using the provided port number.
/// Port number must be between 1024 and 65535 (inclusive).
StartStatus start_server(int port, ShareEnableParams* enable_params);
/// Wonder what this does ?
void stop_server();

#endif // __SERVER_H__
