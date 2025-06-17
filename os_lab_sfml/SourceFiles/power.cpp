#include <gtk/gtk.h>
#include <gst/gst.h>
#include "globals.h"

GtkWidget* shutdown_window = NULL;

gboolean play_shutdown_sound(gpointer data) {
    GstElement* pipeline = gst_parse_launch("playbin uri=file:///C:/Users/samif/source/repos/os_lab_sfml/os_lab_sfml/shutdown.mp3", NULL);
    gst_element_set_state(pipeline, GST_STATE_PLAYING);
    return FALSE;
}

gboolean shut_down(gpointer data) {
    // Get all top-level windows currently open
    GList* windows = gtk_window_list_toplevels();

    // Destroy each one
    for (GList* l = windows; l != NULL; l = l->next) {
        GtkWindow* win = GTK_WINDOW(l->data);
        gtk_widget_destroy(GTK_WIDGET(win));
    }

    // Free the list
    g_list_free(windows);

    // Quit the GTK main loop
    gtk_main_quit();

    return FALSE; 
}


// Separate cleanup + shutdown
gboolean perform_shutdown_cleanup(gpointer data) {
    if (shutdown_window) {
        gtk_widget_destroy(shutdown_window);
        shutdown_window = NULL;
    }

    shut_down(NULL);  // Your actual shutdown call
    return FALSE;
}


void show_shutdown_screen() {
    // Step 2: Create new full-screen shutdown window
    shutdown_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_decorated(GTK_WINDOW(shutdown_window), FALSE);
    gtk_window_fullscreen(GTK_WINDOW(shutdown_window));
    gtk_window_set_title(GTK_WINDOW(shutdown_window), "Shutting Down");
    gtk_window_set_resizable(GTK_WINDOW(shutdown_window), FALSE);

    // Step 3: Create overlay container
    GtkWidget* overlay = gtk_overlay_new();
    gtk_container_add(GTK_CONTAINER(shutdown_window), overlay);

    // Step 4: Black background
    GtkWidget* black_bg = gtk_event_box_new();
    gtk_widget_set_name(black_bg, "shutdown_black");
    gtk_container_add(GTK_CONTAINER(overlay), black_bg);

    // Step 5: Load and center GIF
    GtkWidget* gif = gtk_image_new_from_file("shutdown.gif");
    GtkWidget* center_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(center_box), gif, TRUE, TRUE, 0);
    gtk_widget_set_halign(center_box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(center_box, GTK_ALIGN_CENTER);
    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), center_box);

    // Step 6: Apply black background CSS
    GtkCssProvider* css = gtk_css_provider_new();
    gtk_css_provider_load_from_data(css,
        "#shutdown_black { background-color: black; }",
        -1, NULL);
    GtkStyleContext* ctx = gtk_widget_get_style_context(black_bg);
    gtk_style_context_add_provider(ctx, GTK_STYLE_PROVIDER(css), GTK_STYLE_PROVIDER_PRIORITY_USER);

    // Step 7: Show shutdown screen
    gtk_widget_show_all(shutdown_window);

    // Step 8: Play sound after short delay
    g_timeout_add(500, play_shutdown_sound, NULL);

    // Step 9: Shutdown after 4 seconds
    g_timeout_add_seconds(4, perform_shutdown_cleanup, NULL);
}


