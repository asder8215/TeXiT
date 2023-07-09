#include "server.h"
#include <signal.h>

volatile sig_atomic_t running = true;

StartServerStatus start_server(unsigned int port) {
    if (port < PORT_MIN || port > PORT_MAX) {
        return InvalidPort;
    }
}