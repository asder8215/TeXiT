#include "gui.h"

int main(int argc, char **argv) {
    AdwApplication *app;
    int status;

    app = adw_application_new("me.Asder8215.TeXiT", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(main_window), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}
