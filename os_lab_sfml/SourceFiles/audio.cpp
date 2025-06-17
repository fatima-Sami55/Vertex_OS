#include <gtk/gtk.h>
#include <gst/gst.h>
#include "globals.h"

static GstElement* audio_pipeline = NULL;
static GtkWidget* player_window = NULL;
static GtkWidget* play_button = NULL;
static GtkWidget* pause_button = NULL;
static GtkWidget* status_label = NULL;

static void set_status_text(const gchar* text) {
    if (GTK_IS_LABEL(status_label)) gtk_label_set_text(GTK_LABEL(status_label), text);
}

static void play_audio() {
    if (!audio_pipeline) return;
    if (gst_element_set_state(audio_pipeline, GST_STATE_PLAYING) == GST_STATE_CHANGE_FAILURE) {
        g_printerr("Failed to play\n");
        set_status_text("Status: Error Playing");
    }
    else {
        set_status_text("Status: Playing");
    }
}

static void pause_audio() {
    if (!audio_pipeline) return;
    if (gst_element_set_state(audio_pipeline, GST_STATE_PAUSED) == GST_STATE_CHANGE_FAILURE) {
        g_printerr("Failed to pause\n");
        set_status_text("Status: Error Pausing");
    }
    else {
        set_status_text("Status: Paused");
    }
}

static gboolean bus_call(GstBus* bus, GstMessage* msg, gpointer data) {
    switch (GST_MESSAGE_TYPE(msg)) {
    case GST_MESSAGE_ERROR: {
        GError* err;
        gst_message_parse_error(msg, &err, NULL);
        g_printerr("Error: %s\n", err->message);
        set_status_text("Status: Playback Error");
        g_error_free(err);
        if (audio_pipeline) gst_element_set_state(audio_pipeline, GST_STATE_NULL);
        break;
    }
    case GST_MESSAGE_EOS:
        set_status_text("Status: End of Playback");
        gst_element_set_state(audio_pipeline, GST_STATE_READY);
        break;
    default:
        break;
    }
    return TRUE;
}

static void cleanup_player() {
    if (audio_pipeline) {
        gst_element_set_state(audio_pipeline, GST_STATE_NULL);
        gst_object_unref(audio_pipeline);
        audio_pipeline = NULL;
    }
    if (player_window) {
        gtk_widget_destroy(player_window);
        player_window = NULL;
        status_label = NULL;
        play_button = NULL;
        pause_button = NULL;
    }
}

static void create_audio_player_window(const gchar* uri, const gchar* filename) {
    cleanup_player();

    GError* error = NULL;
    gchar* pipeline_desc = g_strdup_printf("playbin uri=%s", uri);
    audio_pipeline = gst_parse_launch(pipeline_desc, &error);
    g_free(pipeline_desc);

    if (!audio_pipeline) {
        g_printerr("Pipeline error: %s\n", error ? error->message : "unknown");
        if (error) g_error_free(error);
        return;
    }

    GstBus* bus = gst_element_get_bus(audio_pipeline);
    gst_bus_add_watch(bus, bus_call, NULL);
    gst_object_unref(bus);
    runningApps.push_back({ "audio_player", 50, 7 });
    player_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(player_window), "Simple MP3 Player");
    gtk_window_set_default_size(GTK_WINDOW(player_window), 300, 150);
    gtk_window_set_position(GTK_WINDOW(player_window), GTK_WIN_POS_CENTER);
    g_signal_connect(player_window, "destroy", G_CALLBACK(+[](GtkWidget*, gpointer) {
        cleanup_player(); // your audio shutdown code
        close_simulated_app("audio_player"); 
        }), NULL);

    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);
    gtk_container_add(GTK_CONTAINER(player_window), vbox);

    gchar* label_text = g_strdup_printf("Now Playing: %s", filename);
    gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new(label_text), FALSE, FALSE, 5);
    g_free(label_text);

    status_label = gtk_label_new("Status: Playing");
    gtk_box_pack_start(GTK_BOX(vbox), status_label, FALSE, FALSE, 5);

    GtkWidget* hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);

    // Create image buttons instead of text buttons
    play_button = gtk_button_new();
    pause_button = gtk_button_new();

    GtkWidget* play_image = gtk_image_new_from_file("play.png");
    GtkWidget* pause_image = gtk_image_new_from_file("pause.png");

    gtk_button_set_image(GTK_BUTTON(play_button), play_image);
    gtk_button_set_image(GTK_BUTTON(pause_button), pause_image);

    gtk_box_pack_start(GTK_BOX(hbox), play_button, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(hbox), pause_button, TRUE, TRUE, 5);

    g_signal_connect(play_button, "clicked", G_CALLBACK(play_audio), NULL);
    g_signal_connect(pause_button, "clicked", G_CALLBACK(pause_audio), NULL);

    gtk_widget_show_all(player_window);

    if (gst_element_set_state(audio_pipeline, GST_STATE_PLAYING) == GST_STATE_CHANGE_FAILURE) {
        g_printerr("Failed to start playing\n");
        set_status_text("Status: Error Playing");
    }
}

void launch_audio() {
    GtkWidget* dialog = gtk_file_chooser_dialog_new("Select Audio File", NULL,
        GTK_FILE_CHOOSER_ACTION_OPEN, "_Cancel", GTK_RESPONSE_CANCEL,
        "_Open", GTK_RESPONSE_ACCEPT, NULL);

    GtkFileFilter* filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "MP3 Audio");
    gtk_file_filter_add_pattern(filter, "*.mp3");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        gchar* uri = g_filename_to_uri(filename, NULL, NULL);

        create_audio_player_window(uri, g_path_get_basename(filename));

        g_free(uri);
        g_free(filename);
    }

    gtk_widget_destroy(dialog);
}
