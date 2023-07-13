#ifndef __SERVER_H_
#define __SERVER_H_

#define PORT_MIN 1024
#define PORT_MAX 49151
#define DEFAULT_PORT 33378

typedef enum  {
    Success,
    AlreadyStarted,
    InvalidPort,
    Other,
} StartServerStatus;

/// Tries to start a server using the provided port number.
/// Port number must be between 1024 and 49151 (inclusive).
StartServerStatus start_server(int port);
/// Wonder what this does ?
void stop_server();

#endif // __SERVER_H_