#include <gtk/gtk.h>

int main (int argc, char *argv[]){

    //we need to initialize all these functions so that gtk knows
    //to be thread-aware
    if (!g_thread_supported ()){ g_thread_init(NULL); }
    gdk_threads_init();
    gdk_threads_enter();

    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(window);

    gtk_main();
    gdk_threads_leave();

    return 0;
}
