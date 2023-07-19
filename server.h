#ifndef __SERVER_H__
#define __SERVER_H__

#define PORT_MIN 1024
#define PORT_MAX 49151
#define DEFAULT_PORT 33378
#define MAX_CONNECTIONS 3

typedef enum  {
    Success,
    AlreadyStarted,
    InvalidPort,
    Other,
} StartStatus;

/// Tries to start a server using the provided port number.
/// Port number must be between 1024 and 65535 (inclusive).
StartStatus start_server(int port);
/// Wonder what this does ?
void stop_server();

#endif // __SERVER_H__
