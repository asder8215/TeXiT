#ifndef __SERVER_H_
#define __SERVER_H_

#include <stdbool.h>

#define PORT_MIN 1024
#define PORT_MAX 65535

typedef enum  {
    Success,
    InvalidPort,
    BindError,
    Other,
} StartServerStatus;

/// Tries to start a server using the provided port number.
StartServerStatus start_server(unsigned int port);

#endif // __SERVER_H_