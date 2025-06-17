#include <gtk/gtk.h>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <thread>
#include <cstdlib>
#include "launch.h"
#include "globals.h"
#include <gst/gst.h>

void transition_to_home();
void show_password_prompt();

gboolean on_window_resize(GtkWidget* widget, GdkEvent* event, gpointer user_data) {
    if (!originalBackground || !bgImageWidget) return FALSE;

    int newWidth, newHeight;
    gtk_window_get_size(GTK_WINDOW(widget), &newWidth, &newHeight);

    GdkPixbuf* resized = gdk_pixbuf_scale_simple(originalBackground, newWidth, newHeight, GDK_INTERP_BILINEAR);
    gtk_image_set_from_pixbuf(GTK_IMAGE(bgImageWidget), resized);

    return FALSE;
}

gboolean cleanup_gst_pipeline(gpointer data) {
    GstElement* pipeline = (GstElement*)data;
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    return FALSE;
}

gboolean transition_to_home_and_play_sound(gpointer data) {
    // 🔁 Call your original home transition
    transition_to_home();

    // 🎵 Play the starting sound (non-blocking)
    GstElement* pipeline = gst_parse_launch("playbin uri=file:///C:/Users/samif/source/repos/os_lab_sfml/os_lab_sfml/startup.mp3", NULL);
    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    // 🧼 Optional: auto-clean the GStreamer pipeline after playback ends
    g_timeout_add_seconds(5, (GSourceFunc)cleanup_gst_pipeline, pipeline);

    return FALSE;
}

gboolean show_password_screen_wrapper(gpointer data) {
    show_password_prompt();
    return FALSE;
}

// 🔧 Helper: Set image (boot animation or static)
void set_image(const char* filename, gboolean is_animation) {
    if (image) {
        gtk_container_remove(GTK_CONTAINER(mainContainer), image);
    }

    if (is_animation) {
        GdkPixbufAnimation* anim = gdk_pixbuf_animation_new_from_file(filename, NULL);
        image = gtk_image_new_from_animation(anim);
    }
    else {
        image = gtk_image_new_from_file(filename);
    }

    gtk_box_pack_start(GTK_BOX(mainContainer), image, TRUE, TRUE, 0);
    gtk_widget_show_all(window);
}

// 🎯 Final screen: Home UI
void transition_to_home() {
    clear_overlay_widgets();
    set_home_background();
    add_time_label();
    add_home_toggle_icon();
    add_calculator_icon();
    add_calendar_icon();
    add_Minigame_icon();
    add_random_number();
    add_notepad_icon();
    add_fileNest_icon();
    add_music_player_icon();
    add_video_player_icon();
    add_task_player_icon();
    add_shutDown_icon();

    gtk_widget_show_all(window);
    g_signal_connect(window, "configure-event", G_CALLBACK(on_window_resize), NULL);
}

// ⏱️ After GIF, show welcome image
gboolean show_welcome(gpointer data) {
    set_image("welcome.png", FALSE);

    // After 3 seconds, show the password prompt
    g_timeout_add_seconds(3, (GSourceFunc)show_password_screen_wrapper, NULL);
    return FALSE;
}

// 🌀 After logo, show loading GIF
gboolean show_loading(gpointer data) {
    set_image("loading3.gif", TRUE);
    g_timeout_add_seconds(9, show_welcome, NULL);
    return FALSE;
}

// 🚀 ENTRY: Called from main
void start_vertex_ui() {
    // 🪟 Window setup
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "VERTEX");
    gtk_window_set_default_size(GTK_WINDOW(window), 1600, 1000);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_widget_set_app_paintable(window, TRUE);

    // 🎛️ Overlay setup
    overlay = gtk_overlay_new();
    gtk_container_add(GTK_CONTAINER(window), overlay);

    // 📦 Boot container box
    mainContainer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_name(mainContainer, "mainbox");

    cssProvider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(cssProvider,
        "window, #mainbox { background-color: black; }", -1, NULL);

    GtkStyleContext* context = gtk_widget_get_style_context(mainContainer);
    gtk_style_context_add_provider(context,
        GTK_STYLE_PROVIDER(cssProvider),
        GTK_STYLE_PROVIDER_PRIORITY_USER);

    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), mainContainer);

    // 🔰 Show logo first
    image = gtk_image_new_from_file("vertex.png");
    gtk_box_pack_start(GTK_BOX(mainContainer), image, TRUE, TRUE, 0);

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_widget_show_all(window);

    // ⏱️ Boot sequence timer
    g_timeout_add_seconds(2, show_loading, NULL);
}

void on_password_entered(GtkButton* button, gpointer user_data) {
    GtkEntry* entry = GTK_ENTRY(user_data);
    const gchar* input = gtk_entry_get_text(entry);
    const gchar* correct_password = "vertex123";

    if (g_strcmp0(input, correct_password) == 0) {
        // ✅ Password is correct
        GtkWidget* pwd_window = gtk_widget_get_toplevel(GTK_WIDGET(entry));
        gtk_widget_destroy(pwd_window);
        transition_to_home_and_play_sound(NULL);
    }
    else {
        // ❌ Wrong password
        GtkWidget* dialog = gtk_message_dialog_new(GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(entry))),
            GTK_DIALOG_DESTROY_WITH_PARENT,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_OK,
            "❌ Incorrect password. Please try again.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
    }
}

void show_password_prompt() {
    // Step 1: Create a fullscreen window
    GtkWidget* pwd_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_decorated(GTK_WINDOW(pwd_window), FALSE);
    gtk_window_fullscreen(GTK_WINDOW(pwd_window));
    gtk_window_set_title(GTK_WINDOW(pwd_window), "Authentication");

    // Step 2: Create main overlay layout
    GtkWidget* overlay = gtk_overlay_new();
    gtk_container_add(GTK_CONTAINER(pwd_window), overlay);

    // Step 3: Background Image (fullscreen)
    GtkWidget* bg_img = gtk_image_new_from_file("background.jpg"); // or .png
    gtk_widget_set_halign(bg_img, GTK_ALIGN_FILL);
    gtk_widget_set_valign(bg_img, GTK_ALIGN_FILL);
    gtk_widget_set_size_request(bg_img, 1920, 1080); // adjust as needed
    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), bg_img);

    // Step 4: Semi-transparent login box (centered)
    GtkWidget* login_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    gtk_widget_set_size_request(login_box, 400, 300);
    gtk_widget_set_name(login_box, "login_box");

    GtkWidget* label = gtk_label_new("Enter Password");
    gtk_widget_set_name(label, "login_label");

    GtkWidget* entry = gtk_entry_new();
    gtk_entry_set_visibility(GTK_ENTRY(entry), FALSE);
    gtk_entry_set_invisible_char(GTK_ENTRY(entry), '*');
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Password");

    GtkWidget* button = gtk_button_new_with_label("Login");

    gtk_box_pack_start(GTK_BOX(login_box), label, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(login_box), entry, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(login_box), button, FALSE, FALSE, 20);

    // Center the login box using widget alignment props
    gtk_widget_set_halign(login_box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(login_box, GTK_ALIGN_CENTER);

    // Add to overlay directly
    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), login_box);

    // Step 5: CSS for sexy styling
    GtkCssProvider* css = gtk_css_provider_new();
    gtk_css_provider_load_from_data(css,
        "#login_box {"
        "  background-color: rgba(0,0,0,0.5);"
        "  padding: 30px;"
        "  border-radius: 20px;"
        "}"
        "#login_label {"
        "  color: white;"
        "  font-size: 24px;"
        "  font-weight: bold;"
        "}"
        "entry {"
        "  font-size: 18px;"
        "  padding: 10px;"
        "  border-radius: 10px;"
        "}"
        "button {"
        "  font-size: 18px;"
        "  background-color: #1E90FF;"
        "  color: black;"
        "  border-radius: 10px;"
        "  padding: 10px 20px;"
        "}"
        , -1, NULL);

    GtkStyleContext* context = gtk_widget_get_style_context(login_box);
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(css), GTK_STYLE_PROVIDER_PRIORITY_USER);
    gtk_style_context_add_provider(gtk_widget_get_style_context(label), GTK_STYLE_PROVIDER(css), GTK_STYLE_PROVIDER_PRIORITY_USER);
    gtk_style_context_add_provider(gtk_widget_get_style_context(entry), GTK_STYLE_PROVIDER(css), GTK_STYLE_PROVIDER_PRIORITY_USER);
    gtk_style_context_add_provider(gtk_widget_get_style_context(button), GTK_STYLE_PROVIDER(css), GTK_STYLE_PROVIDER_PRIORITY_USER);

    // Step 6: Connect login button
    g_signal_connect(button, "clicked", G_CALLBACK(on_password_entered), entry);

    // Step 7: Show the window
    gtk_widget_show_all(pwd_window);
}
