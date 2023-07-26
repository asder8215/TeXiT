#ifndef __UTIL_H__
#define __UTIL_H__

#include <gdk/gdk.h>
#include <stdbool.h>

#define PORT_MIN 1024
#define PORT_MAX 49151
#define DEFAULT_PORT 33378

typedef enum  {
    Success,
    AlreadyStarted,
    InvalidPort,
    Other,
} StartStatus;

/// Can return NULL.
/// Function takes ownership of **channel** (could be freed), so don't use after this call.
/// **connection** is closed with the **channel**.
/// **closed** is set to `true` if channel and conneciton were closed.
/// Channel can be closed if reached EOF or Error.
/// 
/// Return value must be `g_free`d.
const char* read_channel(GIOChannel* channel, bool* closed);
/// When the channel is closed, so is the connection (at least thats what it seems like).
void close_connection(GIOChannel* channel);

#endif // __UTIL_H__