#include <gtk/gtk.h>
#include <gst/gst.h>
#include <gst/video/videooverlay.h>
#include <gdk/gdkwin32.h>
#include "globals.h"

static GstElement* video_pipeline = NULL;
static GtkWidget* video_window = NULL;
static GtkWidget* play_button = NULL;
static GtkWidget* pause_button = NULL;
static GtkWidget* status_label = NULL;
static GtkWidget* video_area = NULL;
static GtkWidget* seek_slider = NULL;
static gboolean is_seeking = FALSE;


static void set_status_text(const gchar* text) {
    if (GTK_IS_LABEL(status_label)) gtk_label_set_text(GTK_LABEL(status_label), text);
}

static void play_video() {
    if (!video_pipeline) return;
    g_print("Setting pipeline state to PLAYING\n");
    GstStateChangeReturn ret = gst_element_set_state(video_pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr("Failed to play video\n");
        set_status_text("Status: Error Playing");
    }
    else {
        set_status_text("Status: Playing");
    }
}

static void pause_video() {
    if (!video_pipeline) return;
    g_print("Setting pipeline state to PAUSED\n");
    GstStateChangeReturn ret = gst_element_set_state(video_pipeline, GST_STATE_PAUSED);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr("Failed to pause video\n");
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
        if (video_pipeline) gst_element_set_state(video_pipeline, GST_STATE_NULL);
        break;
    }
    case GST_MESSAGE_EOS:
        set_status_text("Status: End of Playback");
        gst_element_set_state(video_pipeline, GST_STATE_READY);
        break;
    default:
        break;
    }
    return TRUE;
}

static void cleanup_player() {
    if (video_pipeline) {
        gst_element_set_state(video_pipeline, GST_STATE_NULL);
        gst_object_unref(video_pipeline);
        video_pipeline = NULL;
    }
    if (video_window) {
        gtk_widget_destroy(video_window);
        video_window = NULL;
        status_label = NULL;
        play_button = NULL;
        pause_button = NULL;
        video_area = NULL;
    }
}

static gboolean update_seek_slider(gpointer user_data) {
    if (!video_pipeline || is_seeking) return TRUE;

    gint64 pos = GST_CLOCK_TIME_NONE;
    gint64 duration = GST_CLOCK_TIME_NONE;

    if (!gst_element_query_position(video_pipeline, GST_FORMAT_TIME, &pos) ||
        !gst_element_query_duration(video_pipeline, GST_FORMAT_TIME, &duration))
        return TRUE;

    if (duration > 0) {
        gdouble fraction = (gdouble)pos / (gdouble)duration * 100.0;
        gtk_range_set_value(GTK_RANGE(seek_slider), fraction);
    }

    return TRUE;
}

static void realize_cb(GtkWidget* widget, gpointer user_data) {
    if (!video_pipeline) return;

    GdkWindow* gdk_window = gtk_widget_get_window(widget);
    if (!GDK_IS_WINDOW(gdk_window)) return;

    guintptr handle = (guintptr)GDK_WINDOW_HWND(gdk_window);
    g_print("Setting video overlay window handle: %p\n", (void*)handle);

    GstVideoOverlay* overlay = GST_VIDEO_OVERLAY(video_pipeline);
    gst_video_overlay_set_window_handle(overlay, handle);
    gst_video_overlay_prepare_window_handle(overlay);
}

void create_video_player_window(const gchar* uri, const gchar* filename) {
    cleanup_player();

    GError* error = NULL;
    gchar* pipeline_desc = g_strdup_printf("playbin uri=%s", uri);
    video_pipeline = gst_parse_launch(pipeline_desc, &error);
    g_free(pipeline_desc);

    if (!video_pipeline) {
        g_printerr("Failed to create pipeline: %s\n", error ? error->message : "unknown");
        if (error) g_error_free(error);
        return;
    }

    GstElement* video_sink = gst_element_factory_make("autovideosink", NULL);
    if (video_sink)
        g_object_set(video_pipeline, "video-sink", video_sink, NULL);

    GstBus* bus = gst_element_get_bus(video_pipeline);
    gst_bus_add_watch(bus, bus_call, NULL);
    gst_object_unref(bus);

    runningApps.push_back({ "video_player", 200, 8 });
    video_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(video_window), "Simple MP4 Player");
    gtk_window_set_default_size(GTK_WINDOW(video_window), 640, 480);
    gtk_window_set_position(GTK_WINDOW(video_window), GTK_WIN_POS_CENTER);
    g_signal_connect(video_window, "destroy", G_CALLBACK(+[](GtkWidget*, gpointer) {
        close_simulated_app("video_player");
        cleanup_player();
        }), NULL);

    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(video_window), vbox);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 0);


    gchar* label_text = g_strdup_printf("Now Playing: %s", filename);
    gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new(label_text), FALSE, FALSE, 5);
    g_free(label_text);

    status_label = gtk_label_new("Status: Playing");
    gtk_box_pack_start(GTK_BOX(vbox), status_label, FALSE, FALSE, 5);

    seek_slider = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 100, 1);
    gtk_range_set_value(GTK_RANGE(seek_slider), 0);
    gtk_box_pack_start(GTK_BOX(vbox), seek_slider, FALSE, FALSE, 5);

    video_area = gtk_drawing_area_new();
    gtk_widget_set_hexpand(video_area, TRUE);
    gtk_widget_set_vexpand(video_area, TRUE);
    gtk_box_pack_start(GTK_BOX(vbox), video_area, TRUE, TRUE, 5);

    g_signal_connect(video_area, "realize", G_CALLBACK(realize_cb), NULL);
    g_signal_connect(video_area, "size-allocate", G_CALLBACK(+[](GtkWidget* widget, GdkRectangle* allocation, gpointer) {
        if (!video_pipeline) return;

        GstVideoOverlay* overlay = GST_VIDEO_OVERLAY(video_pipeline);
        if (!overlay) return;

        GdkWindow* gdk_window = gtk_widget_get_window(widget);
        if (!GDK_IS_WINDOW(gdk_window)) return;

        guintptr handle = (guintptr)GDK_WINDOW_HWND(gdk_window);
        gst_video_overlay_set_window_handle(overlay, handle);

        // Manually force the video up by setting y = 0
        // Instead of allocation->y, which might be >0 due to top padding
        gst_video_overlay_set_render_rectangle(
            overlay,
            allocation->x,
            0, // Pushed to the top
            allocation->width,
            allocation->height
        );

        gst_video_overlay_expose(overlay);
        }), NULL);
    g_signal_connect(seek_slider, "value-changed", G_CALLBACK(+[](GtkRange* range, gpointer) {
        return;
        }), NULL);
    g_signal_connect(seek_slider, "button-release-event", G_CALLBACK(+[](GtkWidget*, GdkEvent*, gpointer) {
        if (!video_pipeline) return FALSE;

        gint64 duration = GST_CLOCK_TIME_NONE;
        if (!gst_element_query_duration(video_pipeline, GST_FORMAT_TIME, &duration)) return FALSE;

        gdouble val = gtk_range_get_value(GTK_RANGE(seek_slider));
        gint64 seek_pos = (gint64)((val / 100.0) * duration);

        is_seeking = TRUE;

        gst_element_seek_simple(video_pipeline, GST_FORMAT_TIME,
            (GstSeekFlags)(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT), seek_pos);

        g_timeout_add(300, (GSourceFunc)+[](gpointer) -> gboolean {
            is_seeking = FALSE;
            return G_SOURCE_REMOVE;
            }, NULL);

        return FALSE;
        }), NULL);



    GtkWidget* hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);

    play_button = gtk_button_new();
    pause_button = gtk_button_new();

    GdkPixbuf* play_pixbuf = gdk_pixbuf_new_from_file_at_size("play.png", 100, 100, NULL);
    GdkPixbuf* pause_pixbuf = gdk_pixbuf_new_from_file_at_size("pause.png", 100, 100, NULL);

    GtkWidget* play_image = gtk_image_new_from_pixbuf(play_pixbuf);
    GtkWidget* pause_image = gtk_image_new_from_pixbuf(pause_pixbuf);

    g_object_unref(play_pixbuf);
    g_object_unref(pause_pixbuf);

    gtk_button_set_image(GTK_BUTTON(play_button), play_image);
    gtk_button_set_image(GTK_BUTTON(pause_button), pause_image);

    gtk_box_pack_start(GTK_BOX(hbox), play_button, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(hbox), pause_button, TRUE, TRUE, 5);

    g_signal_connect(play_button, "clicked", G_CALLBACK(play_video), NULL);
    g_signal_connect(pause_button, "clicked", G_CALLBACK(pause_video), NULL);
    g_signal_connect(seek_slider, "button-press-event", G_CALLBACK(+[](GtkWidget*, GdkEvent*, gpointer) {
        is_seeking = TRUE;
        return FALSE;
        }), NULL);
    g_signal_connect(seek_slider, "button-release-event", G_CALLBACK(+[](GtkWidget*, GdkEvent*, gpointer) {
        is_seeking = FALSE;
        return FALSE;
        }), NULL);


    gtk_widget_show_all(video_window);

    g_timeout_add(500, update_seek_slider, NULL); // updates every 500ms
    play_video(); // auto-start

}

void launch_video() {
    GtkWidget* dialog = gtk_file_chooser_dialog_new("Select Video File", NULL,
        GTK_FILE_CHOOSER_ACTION_OPEN,
        "_Cancel", GTK_RESPONSE_CANCEL,
        "_Open", GTK_RESPONSE_ACCEPT,
        NULL);

    GtkFileFilter* filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "Video Files");
    gtk_file_filter_add_pattern(filter, "*.mp4");
    gtk_file_filter_add_pattern(filter, "*.mkv");
    gtk_file_filter_add_pattern(filter, "*.avi");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        gchar* uri = g_filename_to_uri(filename, NULL, NULL);

        create_video_player_window(uri, g_path_get_basename(filename));

        g_free(uri);
        g_free(filename);
    }

    gtk_widget_destroy(dialog);
}

