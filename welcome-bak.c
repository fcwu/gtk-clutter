#include <gtk/gtk.h>
#include <clutter/clutter.h>
#include <clutter-gtk/clutter-gtk.h>

int image_width, image_height;
static const int screen_width = 800;
static const int screen_height = 600;


static float smooth_step2(float a, float b, float z) {
    return (b - a) * z + a;
}

static void button_clicked(GtkWidget *widget, GdkEvent *event, gpointer data)
{
    void on_complete(GtkWidget *widget, gpointer data)
    {
        g_debug("%s: %d", __FUNCTION__, __LINE__);
        clutter_actor_hide(CLUTTER_ACTOR(data));
    }
    void on_timeline_scale_new_frame(ClutterTimeline *timeline, gint frame_num, gpointer data) {
        float scale = clutter_timeline_get_progress(timeline);
        if (scale >= 1)
            scale = 1;
        if (scale <= 0)
            scale = 0;
        float scale_amount = smooth_step2(1.0, 2.0, scale);
        clutter_actor_set_scale(CLUTTER_ACTOR(data), scale_amount, scale_amount);
    }

    double next_x = (screen_width - image_width * 2) / 2.0;
    double next_y = (screen_height - image_height * 2) / 2.0;
    ClutterActor * actor = CLUTTER_ACTOR(data);
    g_debug("%s: image [%d, %d], screen [%d, %d] next [%.2f, %.2f]",
            __FUNCTION__, image_width, image_height, 
            screen_width, screen_height, next_x, next_y);

    clutter_actor_set_position (actor, 
                                (screen_width - image_width) / 2,
                                (screen_height - image_height) / 2);
    clutter_actor_set_size (actor, image_width, image_height);
    clutter_actor_set_opacity (actor, 255);
    clutter_actor_show(actor);

    ClutterAnimation * animation;
    animation = clutter_actor_animate (actor, CLUTTER_LINEAR, 
                                       1000,
                                       "x", next_x,
                                       "y", next_y,
                                       "opacity", 0,
                                       NULL);
    g_signal_connect(clutter_animation_get_timeline(animation),
                     "new-frame",
                     G_CALLBACK(on_timeline_scale_new_frame),
                     actor);
    g_signal_connect(clutter_animation_get_timeline (animation),
                     "completed",
                     G_CALLBACK (on_complete),
                     actor);
}


static GdkPixbuf *load_pixbuf_from_file (const char *filename)
{
    GError *error = NULL;
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file (filename, &error);

    if (pixbuf == NULL) {
        g_critical("Error loading file: %d : %s\n", error->code, error->message);
        g_error_free(error);
    }
    return pixbuf;
}

static void layout()
{
    struct Button {
        const char * image;
    } my_buttons[] = {
        {"astask/logo0.png"},
        {"astask/logo1.png"},
        {"astask/logo2.png"},
        {"astask/logo3.png"},
        {"astask/logo4.png"}
    };
 
    g_debug("layout");

    GtkWidget *window;
    GtkWidget *fixed;
    GtkWidget *embed;

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), screen_width, screen_height);

    // embed
    embed = gtk_clutter_embed_new();
    gtk_container_add(GTK_CONTAINER(window), embed);
    // actor
    ClutterActor *actor = gtk_clutter_actor_new();
    GtkWidget *bin = gtk_clutter_actor_get_widget (GTK_CLUTTER_ACTOR (actor));
    // stage
    ClutterActor *stage = gtk_clutter_embed_get_stage (GTK_CLUTTER_EMBED (embed));
    clutter_container_add_actor(CLUTTER_CONTAINER(stage), actor);
    clutter_actor_set_size (actor, screen_width, screen_height);


    fixed = gtk_fixed_new();
    gtk_container_add(GTK_CONTAINER(bin), fixed);

    GtkWidget * w = gtk_event_box_new();
    GtkWidget * image = gtk_image_new_from_file(my_buttons[0].image);

    const GdkPixbuf *pb = gtk_image_get_pixbuf(GTK_IMAGE(image));
    image_width = gdk_pixbuf_get_width(pb);
    image_height = gdk_pixbuf_get_height(pb);

    gtk_container_add(GTK_CONTAINER(w), image);
    gtk_fixed_put(GTK_FIXED(fixed), w, (screen_width - image_width) / 2,
                  (screen_height - image_height) / 2);


    // animation
    actor = gtk_clutter_actor_new();
    GtkWidget *gtk_frame_container = gtk_clutter_actor_get_widget (GTK_CLUTTER_ACTOR (actor));
    gtk_container_add (GTK_CONTAINER (gtk_frame_container), gtk_image_new_from_file(my_buttons[0].image));
    clutter_container_add_actor(CLUTTER_CONTAINER (stage), actor);
    clutter_actor_hide(actor);

    gtk_widget_show_all(GTK_WIDGET(w));
    gtk_widget_show_all(GTK_WIDGET(fixed));


    g_signal_connect(G_OBJECT(w), "button-press-event",
                     G_CALLBACK(button_clicked), 
                     actor);

    g_signal_connect_swapped(G_OBJECT(window), "destroy",
                             G_CALLBACK(gtk_main_quit), G_OBJECT(window));

    gtk_widget_show_all(window); 
}

int main(int argc, char *argv[])
{
    //we need to initialize all these functions so that gtk knows
    //to be thread-aware
    if (!g_thread_supported()) { 
        g_thread_init(NULL); 
    }
    gdk_threads_init();
    gdk_threads_enter();

    g_assert(gtk_clutter_init(&argc, &argv) == CLUTTER_INIT_SUCCESS);

    layout();

    gtk_main();
    gdk_threads_leave();

    return 0;
}
