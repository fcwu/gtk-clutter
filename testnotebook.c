#include <gtk/gtk.h>
#include <clutter/clutter.h>
#include <clutter-gtk/clutter-gtk.h>

ClutterActor *actor_frames[5];
float anglemax[4] = {-160., -140., -120., -100.};
guint old_page_to_hide;


gdouble
on_alpha(ClutterAlpha *alpha, gpointer data)
{
    ClutterTimeline *timeline = clutter_alpha_get_timeline (alpha);
    return clutter_timeline_get_progress (timeline);
}


void
fade_tab(GtkNotebook     *notebook,
         GtkNotebookPage *page,
         guint            page_num,
         gpointer         user_data)
{
    guint old_page_num;
    old_page_num = gtk_notebook_get_current_page (notebook);

    if(old_page_num == -1)
        return;

    // fade in the new page and fade out the old one
    clutter_actor_set_opacity (actor_frames[old_page_num],255);
    clutter_actor_set_opacity (actor_frames[page_num],0);
    clutter_actor_show (actor_frames[old_page_num]);
    clutter_actor_show (actor_frames[page_num]);

    ClutterAnimation *animation_out = clutter_actor_animate (actor_frames[old_page_num],
            CLUTTER_LINEAR, 1000,
            "opacity", 0,
            NULL);
    ClutterAnimation *animation_in = clutter_actor_animate (actor_frames[page_num],
            CLUTTER_LINEAR, 1000,
            "opacity", 255,
            NULL);

    // add move translation
    if (user_data != NULL) {
        ClutterTimeline *timeline = clutter_timeline_new(1000 /* milliseconds */);
        clutter_timeline_start(timeline);
        ClutterAlpha* alpha = clutter_alpha_new_with_func (timeline, &on_alpha, NULL, NULL);
        ClutterBehaviour *behaviour;


        if((char *)user_data == "one"){

            ClutterKnot knot[2];
            knot[0].x = 101;
            knot[0].y = 151;
            knot[1].x = 101;
            knot[1].y = 76;  
            behaviour = clutter_behaviour_path_new_with_knots(alpha, knot, 
                            sizeof(knot) / sizeof(ClutterKnot));
            clutter_behaviour_apply(behaviour, actor_frames[page_num]);

        } else {
            // old frame
            ClutterKnot knot[3];
            knot[0].x = 101;
            knot[0].y = 76;
            knot[1].x = 101;
            knot[1].y = 131;
            knot[2].x = 101;
            knot[2].y = 76;
            behaviour = clutter_behaviour_path_new_with_knots (alpha, knot, 
                            sizeof(knot) / sizeof(ClutterKnot));
            clutter_behaviour_apply (behaviour, actor_frames[old_page_num]);            

            // new frame
            knot[1].y = 21;
            behaviour = clutter_behaviour_path_new_with_knots (alpha, knot, 
                            sizeof(knot) / sizeof(ClutterKnot));
            clutter_behaviour_apply (behaviour, actor_frames[page_num]);
        }

        g_object_unref (timeline);
    }

    g_signal_connect_swapped (clutter_animation_get_timeline (animation_out),
            "completed",
            G_CALLBACK (clutter_actor_hide),
            actor_frames[old_page_num]);
    g_signal_connect_swapped (clutter_animation_get_timeline (animation_in),
            "completed",
            G_CALLBACK (clutter_actor_hide),
            actor_frames[page_num]);

}

void
end_rotation(ClutterActor *actor){
    // hide old page num in addition to actor, which is actor
    // (to avoid seing the new before the animation is finished)
    clutter_actor_hide(actor_frames[old_page_to_hide]);
    clutter_actor_hide(actor);
}

void
rotate_tab (GtkNotebook     *notebook,
            GtkNotebookPage *page,
            guint            page_num,
            gpointer         user_data)
{
    guint old_page_num;
    int i;
    char *rotation_sense;
    ClutterVertex *center = clutter_vertex_new(0., 75., 0.);
    old_page_num = gtk_notebook_get_current_page (notebook);
    old_page_to_hide = old_page_num;

    if ((char *)user_data == "left")
        rotation_sense = "rotation-angle-y";
    else
        rotation_sense = "rotation-angle-x";

    if (old_page_num == -1)
        return;

    if (page_num > old_page_num) {

        // rotate all the previous page to their max angle
        for (i = old_page_num; i < page_num; i++) {
            // reverse the order depth (set to 0 is ok), cf below
            clutter_actor_set_depth (actor_frames[i], 0);
            clutter_actor_show (actor_frames[i]);
            ClutterAnimation *animation_turn = clutter_actor_animate (actor_frames[i],
                    CLUTTER_EASE_OUT_QUAD, 1000,
                    rotation_sense, anglemax[i],
                    "fixed::rotation-center-x", center,
                    "fixed::rotation-center-y", center,
                    "fixed::rotation-center-z", center,
                    NULL);
        }
    } else {
        // rotate all the previous page to 0 and then hide them
        // show current actor frame to avoid seing the new tab (internal to gtknotebook)
        // before the end of animation
        clutter_actor_show(actor_frames[old_page_num]);
        clutter_actor_set_depth (actor_frames[old_page_num], old_page_num);
        int page_dest = page_num-1;
        for(i = old_page_num; i > page_dest; i--) {
            // set the depth so that the previous frame hide the last one (is there an order statement???)
            clutter_actor_set_depth (actor_frames[i], 4-i);
            ClutterAnimation *animation_turn = clutter_actor_animate (actor_frames[i],
                    CLUTTER_EASE_OUT_QUAD, 1000 - i*200,
                    rotation_sense, 0.,
                    NULL);
            g_signal_connect_swapped (clutter_animation_get_timeline (animation_turn),
                    "completed",
                    G_CALLBACK (end_rotation),
                    actor_frames[i]);
        }
    }


}

int
main (int argc, char **argv)
{
    gtk_clutter_init (&argc, &argv);

    GtkWidget *frame;
    GtkWidget *hbox;
    GtkWidget *label;
    GtkWidget *image;

    char *stockImageRef[5];
    stockImageRef[0] = GTK_STOCK_OPEN;
    stockImageRef[1] = GTK_STOCK_PASTE;
    stockImageRef[2] = GTK_STOCK_PREFERENCES;
    stockImageRef[3] = GTK_STOCK_PRINT;
    stockImageRef[4] = GTK_STOCK_QUIT;

    int i;
    char bufferf[32];
    char bufferfr[32];
    char bufferl[32];
    GtkWidget *window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    GtkWidget *embed = gtk_clutter_embed_new ();

    gtk_container_add (GTK_CONTAINER (window), embed);
    gtk_window_set_default_size (GTK_WINDOW (window), 400, 300);

    ClutterActor *stage = gtk_clutter_embed_get_stage (GTK_CLUTTER_EMBED (embed));
    ClutterActor *actor = gtk_clutter_actor_new ();
    GtkWidget *bin = gtk_clutter_actor_get_widget (GTK_CLUTTER_ACTOR (actor));

    GtkWidget *notebook;
    notebook = gtk_notebook_new ();
    gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook), GTK_POS_TOP);
    g_signal_connect (GTK_NOTEBOOK (notebook), "switch-page",
            //                G_CALLBACK (fade_tab), NULL);
                            G_CALLBACK (fade_tab), "one");
    //                 G_CALLBACK (fade_tab), "move");
    //                G_CALLBACK (rotate_tab), &"left");
    //                G_CALLBACK (rotate_tab), &"down");
    gtk_widget_show (notebook);


    /* Let's append a bunch of pages to the notebook */
    for (i = 0; i < 5; i++) {
        sprintf(bufferf, "Frame %d", i + 1);
        sprintf(bufferfr, "Frame Content %d", i + 1);
        sprintf(bufferl, "Page %d", i + 1);

        frame = gtk_frame_new (bufferf);
        gtk_container_set_border_width (GTK_CONTAINER (frame), 10);
        gtk_widget_set_size_request (frame, 100, 75);
        gtk_widget_show (frame);
        image = gtk_image_new_from_stock (stockImageRef[i], GTK_ICON_SIZE_MENU);
        label = gtk_label_new (bufferfr);
        hbox = gtk_hbox_new (TRUE, 0);
        gtk_container_add (GTK_CONTAINER (frame), hbox);
        gtk_container_add (GTK_CONTAINER (hbox), label);
        gtk_container_add (GTK_CONTAINER (hbox), image);
        gtk_widget_show (hbox);
        gtk_widget_show (label);
        gtk_widget_show (image);


        label = gtk_label_new (bufferl);
        gtk_notebook_append_page (GTK_NOTEBOOK (notebook), frame, label);

        // one actor (associated to a gtk_container) for each duplicate frame
        frame = gtk_frame_new (bufferf);
        gtk_container_set_border_width (GTK_CONTAINER (frame), 10);
        gtk_widget_set_size_request (frame, 100, 75);
        gtk_widget_show (frame);
        image = gtk_image_new_from_stock (stockImageRef[i], GTK_ICON_SIZE_MENU);
        label = gtk_label_new (bufferfr);
        hbox = gtk_hbox_new (TRUE, 0);
        gtk_container_add (GTK_CONTAINER (frame), hbox);
        gtk_container_add (GTK_CONTAINER (hbox), label);
        gtk_container_add (GTK_CONTAINER (hbox), image);
        gtk_widget_show (hbox);
        gtk_widget_show (label);
        gtk_widget_show (image);

        actor_frames[i] = gtk_clutter_actor_new ();
        GtkWidget *gtk_frame_container = gtk_clutter_actor_get_widget (GTK_CLUTTER_ACTOR (actor_frames[i]));
        clutter_actor_set_position (actor_frames[i], 101., 76.);
        gtk_container_add (GTK_CONTAINER (gtk_frame_container), frame);

    }

    clutter_actor_set_position (actor, 100., 50.);
    gtk_container_add (GTK_CONTAINER (bin), notebook);
    clutter_container_add_actor (CLUTTER_CONTAINER (stage), actor);

    float width, height;
    clutter_actor_get_size (actor, &width, &height);

    for (i = 0; i < 5; i++) {
        clutter_actor_set_size (actor_frames[i], width-12, height-31);
        clutter_container_add_actor (CLUTTER_CONTAINER (stage), actor_frames[i]);
        clutter_actor_hide(actor_frames[i]);
    }

    gtk_widget_show_all (window);
    gtk_main ();

    return 0;
}
