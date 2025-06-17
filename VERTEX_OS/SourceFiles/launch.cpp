#include <gtk/gtk.h>
#include <ctime>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <thread>
#include <cstdlib>
#include "calc.h" 
#include "calender.h"
#include "globals.h"
#include "Minigame.h"
#include "random.h"
#include "notepad.h"
#include "file.h"
#include "audio.h"
#include "video.h"
#include "power.h"
#include "task.h"

GtkWidget* overlay = NULL;
GtkWidget* window = NULL;
GtkWidget* mainContainer = NULL;
GtkWidget* image = NULL;
GtkCssProvider* cssProvider = NULL;

GdkPixbuf* originalBackground = NULL;
GtkWidget* bgImageWidget = NULL;

int current_home_index = 0;
const char* home_images[] = { "home.png", "home1.png", "home2.jpg", "home4.jpg"};

std::vector<SimulatedApp> runningApps;

int load_wallpaper_index() {
    int index = 0;
    FILE* file = nullptr;
    if (fopen_s(&file, "wallpaper_config.txt", "r") == 0 && file != nullptr) {
        fscanf_s(file, "%d", &index);
        fclose(file);
    }
    return index % 4;
}

void save_wallpaper_index(int index) {
    FILE* file = nullptr;
    if (fopen_s(&file, "wallpaper_config.txt", "w") == 0 && file != nullptr) {
        fprintf(file, "%d", index);
        fclose(file);
    }
    else {
        g_printerr("Failed to save wallpaper index\n");
    }
}

void clear_overlay_widgets() {
    GList* children = gtk_container_get_children(GTK_CONTAINER(overlay));
    for (GList* iter = children; iter != NULL; iter = iter->next) {
        gtk_container_remove(GTK_CONTAINER(overlay), GTK_WIDGET(iter->data));
    }
    g_list_free(children);
}

void set_home_background();

gboolean on_home_icon_click(GtkWidget* widget, GdkEventButton* event, gpointer user_data) {
    current_home_index = (current_home_index + 1) % 4;
    save_wallpaper_index(current_home_index);
    set_home_background();
    return TRUE;
}

gboolean on_calc_icon_click(GtkWidget* widget, GdkEventButton* event, gpointer user_data) {
    if (is_app_running("calculator")){
        show_app_already_running_box("calculator");
    return TRUE;
    }
    runningApps.push_back({ "calculator", 45, 8 });
    launch_calculator();
    return TRUE;
}

gboolean on_calender_icon_click(GtkWidget* widget, GdkEventButton* event, gpointer user_data) {
    if (is_app_running("calendar")) {
        show_app_already_running_box("calendar");
        return TRUE;
    }
    runningApps.push_back({ "calendar", 30, 5 });
    launch_calendar();
    return TRUE;
}

gboolean on_Minigame_icon_click(GtkWidget* widget, GdkEventButton* event, gpointer user_data) {
    if (is_app_running("minigame")) {
        show_app_already_running_box("minigame");
        return TRUE;
    }
    runningApps.push_back({ "minigame", 60, 12 });
    launch_Minigame();
    return TRUE;
}

gboolean on_random_icon_click(GtkWidget* widget, GdkEventButton* event, gpointer user_data) {
    if (is_app_running("random_generator")) {
        show_app_already_running_box("random_generator");
        return TRUE;
    }
    runningApps.push_back({ "random_generator", 25, 4 });
    launch_random();
    return TRUE;
}

gboolean on_notepad_icon_click(GtkWidget* widget, GdkEventButton* event, gpointer user_data) {
    if (is_app_running("notepad")) {
        show_app_already_running_box("notepad");
        return TRUE;
    }
    runningApps.push_back({ "notepad", 35, 6 });
    launch_notepad();
    return TRUE;
}

gboolean on_fileNest_icon_click(GtkWidget* widget, GdkEventButton* event, gpointer user_data) {
    if (is_app_running("filenest")) {
        show_app_already_running_box("filenest");
        return TRUE;
    }
    runningApps.push_back({ "filenest", 55, 10 });
    launch_FileNest();
    return TRUE;
}

gboolean on_audio_icon_click(GtkWidget* widget, GdkEventButton* event, gpointer user_data) {
    if (is_app_running("audio_player")) {
        show_app_already_running_box("audio_player");
        return TRUE;
    }
    launch_audio();
    return TRUE;
}

gboolean on_video_icon_click(GtkWidget* widget, GdkEventButton* event, gpointer user_data) {
    if (is_app_running("video_player")) {
        show_app_already_running_box("video_player");
        return TRUE;
    }
    launch_video();
    return TRUE;
}

gboolean on_task_icon_click(GtkWidget* widget, GdkEventButton* event, gpointer user_data) {
    if (is_app_running("task")) {
        show_app_already_running_box("task");
        return TRUE;
    }
    runningApps.push_back({ "task", 55, 10 });
    launch_task();
    return FALSE;
}

gboolean on_power_icon_click(GtkWidget* widget, GdkEventButton* event, gpointer user_data) {
    show_shutdown_screen();
    clear_overlay_widgets();
    return FALSE;
}



void set_home_background() {
    GError* error = NULL;
    current_home_index = load_wallpaper_index();

    originalBackground = gdk_pixbuf_new_from_file(home_images[current_home_index], &error);
    if (!originalBackground) {
        g_printerr("Failed to load image: %s\n", error->message);
        g_error_free(error);
        return;
    }

    int width, height;
    gtk_window_get_size(GTK_WINDOW(window), &width, &height);
    if (width == 0 || height == 0) {
        width = 800;
        height = 600;
    }

    GdkPixbuf* scaled = gdk_pixbuf_scale_simple(originalBackground, width, height, GDK_INTERP_BILINEAR);

    if (bgImageWidget) {
        gtk_container_remove(GTK_CONTAINER(overlay), bgImageWidget);
        bgImageWidget = NULL;
    }

    bgImageWidget = gtk_image_new_from_pixbuf(scaled);
    gtk_container_add(GTK_CONTAINER(overlay), bgImageWidget);
    gtk_widget_show_all(overlay); 
}

void add_time_label() {
    time_t now = time(0);
    struct tm ltm;

#ifdef _MSC_VER
    localtime_s(&ltm, &now);
#else
    ltm = *localtime(&now);
#endif

    // Build custom formatted time string manually
    std::ostringstream oss;
    const char* months[] = { "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC" };

    int hour = ltm.tm_hour;
    const char* period = "AM";
    if (hour >= 12) {
        period = "PM";
        if (hour > 12) hour -= 12;
    }
    if (hour == 0) hour = 12; // Midnight fix

    oss << ltm.tm_mday << " "
        << months[ltm.tm_mon] << " "
        << (ltm.tm_year + 1900) << " "
        << hour << ":"
        << std::setfill('0') << std::setw(2) << ltm.tm_min << ":"
        << std::setfill('0') << std::setw(2) << ltm.tm_sec << " "
        << period;

    GtkWidget* label = gtk_label_new(oss.str().c_str());

    GtkCssProvider* labelCss = gtk_css_provider_new();
    gtk_css_provider_load_from_data(labelCss,
        "label { color: white; font-size: 20px; font-weight: bold; }", -1, NULL);
    GtkStyleContext* ctx = gtk_widget_get_style_context(label);
    gtk_style_context_add_provider(ctx,
        GTK_STYLE_PROVIDER(labelCss),
        GTK_STYLE_PROVIDER_PRIORITY_USER);

    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), label);
    gtk_widget_set_halign(label, GTK_ALIGN_END);
    gtk_widget_set_valign(label, GTK_ALIGN_END);
    gtk_widget_set_margin_end(label, 30);
    gtk_widget_set_margin_bottom(label, 30);
}

void add_calculator_icon() {
    GError* error = NULL;
    GdkPixbuf* pixbuf = gdk_pixbuf_new_from_file("calc.png", &error);
    if (!pixbuf) {
        g_printerr("Failed to load image: %s\n", error->message);
        g_error_free(error);
        return;
    }

    GdkPixbuf* scaled_icon = gdk_pixbuf_scale_simple(pixbuf, 64, 64, GDK_INTERP_BILINEAR);
    GtkWidget* calcIcon = gtk_image_new_from_pixbuf(scaled_icon);
    g_object_unref(pixbuf);

    GtkWidget* calcButton = gtk_event_box_new();
    gtk_container_add(GTK_CONTAINER(calcButton), calcIcon);

    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), calcButton);
    gtk_widget_set_halign(calcButton, GTK_ALIGN_START);
    gtk_widget_set_valign(calcButton, GTK_ALIGN_START);
    gtk_widget_set_margin_start(calcButton, 30);
    gtk_widget_set_margin_top(calcButton, 30);

    g_signal_connect(calcButton, "button-press-event", G_CALLBACK(on_calc_icon_click), NULL);
}

void add_calendar_icon() {
    GError* error = NULL;
    GdkPixbuf* pixbuf = gdk_pixbuf_new_from_file("calendar.png", &error);
    if (!pixbuf) {
        g_printerr("Failed to load image: %s\n", error->message);
        g_error_free(error);
        return;
    }

    GdkPixbuf* scaled_icon = gdk_pixbuf_scale_simple(pixbuf, 64, 64, GDK_INTERP_BILINEAR);
    GtkWidget* calendarIcon = gtk_image_new_from_pixbuf(scaled_icon);
    g_object_unref(pixbuf);

    GtkWidget* calendarButton = gtk_event_box_new();
    gtk_container_add(GTK_CONTAINER(calendarButton), calendarIcon);

    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), calendarButton);
    gtk_widget_set_halign(calendarButton, GTK_ALIGN_START);
    gtk_widget_set_valign(calendarButton, GTK_ALIGN_START);
    gtk_widget_set_margin_start(calendarButton, 30);
    gtk_widget_set_margin_top(calendarButton, 140); // Below calculator

    g_signal_connect(calendarButton, "button-press-event", G_CALLBACK(on_calender_icon_click), NULL);
}

void add_Minigame_icon() {
    GError* error = NULL;
    GdkPixbuf* pixbuf = gdk_pixbuf_new_from_file("Minigame.png", &error);
    if (!pixbuf) {
        g_printerr("Failed to load image: %s\n", error->message);
        g_error_free(error);
        return;
    }

    GdkPixbuf* scaled_icon = gdk_pixbuf_scale_simple(pixbuf, 64, 64, GDK_INTERP_BILINEAR);
    GtkWidget* MinigameIcon = gtk_image_new_from_pixbuf(scaled_icon);
    g_object_unref(pixbuf);

    GtkWidget* MinigameButton = gtk_event_box_new();
    gtk_container_add(GTK_CONTAINER(MinigameButton), MinigameIcon);

    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), MinigameButton);
    gtk_widget_set_halign(MinigameButton, GTK_ALIGN_START);
    gtk_widget_set_valign(MinigameButton, GTK_ALIGN_START);
    gtk_widget_set_margin_start(MinigameButton, 30);
    gtk_widget_set_margin_top(MinigameButton, 250); // below calendar

    g_signal_connect(MinigameButton, "button-press-event", G_CALLBACK(on_Minigame_icon_click), NULL);
}

void add_random_number() {
        GError* error = NULL;
        GdkPixbuf* pixbuf = gdk_pixbuf_new_from_file("random.png", &error);
        if (!pixbuf) {
            g_printerr("Failed to load image: %s\n", error->message);
            g_error_free(error);
            return;
        }

        GdkPixbuf* scaled_icon = gdk_pixbuf_scale_simple(pixbuf, 64, 64, GDK_INTERP_BILINEAR);
        GtkWidget* randomIcon = gtk_image_new_from_pixbuf(scaled_icon);
        g_object_unref(pixbuf);

        GtkWidget* randomButton = gtk_event_box_new();
        gtk_container_add(GTK_CONTAINER(randomButton), randomIcon);

        gtk_overlay_add_overlay(GTK_OVERLAY(overlay), randomButton);
        gtk_widget_set_halign(randomButton, GTK_ALIGN_START);
        gtk_widget_set_valign(randomButton, GTK_ALIGN_START);
        gtk_widget_set_margin_start(randomButton, 30);
        gtk_widget_set_margin_top(randomButton, 340); // below calendar

        g_signal_connect(randomButton, "button-press-event", G_CALLBACK(on_random_icon_click), NULL);
}

void add_notepad_icon() {
    GError* error = NULL;
    GdkPixbuf* pixbuf = gdk_pixbuf_new_from_file("notepad.png", &error);
    if (!pixbuf) {
        g_printerr("Failed to load image: %s\n", error->message);
        g_error_free(error);
        return;
    }

    GdkPixbuf* scaled_icon = gdk_pixbuf_scale_simple(pixbuf, 64, 64, GDK_INTERP_BILINEAR);
    GtkWidget* notepadIcon = gtk_image_new_from_pixbuf(scaled_icon);
    g_object_unref(pixbuf);

    GtkWidget* notepadButton = gtk_event_box_new();
    gtk_container_add(GTK_CONTAINER(notepadButton), notepadIcon);

    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), notepadButton);
    gtk_widget_set_halign(notepadButton, GTK_ALIGN_START);
    gtk_widget_set_valign(notepadButton, GTK_ALIGN_START);
    gtk_widget_set_margin_start(notepadButton, 30);
    gtk_widget_set_margin_top(notepadButton, 430); // below calendar

    g_signal_connect(notepadButton, "button-press-event", G_CALLBACK(on_notepad_icon_click), NULL);
}

void add_fileNest_icon() {
    GError* error = NULL;
    GdkPixbuf* pixbuf = gdk_pixbuf_new_from_file("file.png", &error);
    if (!pixbuf) {
        g_printerr("Failed to load image: %s\n", error->message);
        g_error_free(error);
        return;
    }

    GdkPixbuf* scaled_icon = gdk_pixbuf_scale_simple(pixbuf, 64, 64, GDK_INTERP_BILINEAR);
    GtkWidget* fileNestIcon = gtk_image_new_from_pixbuf(scaled_icon);
    g_object_unref(pixbuf);

    GtkWidget* fileNestButton = gtk_event_box_new();
    gtk_container_add(GTK_CONTAINER(fileNestButton), fileNestIcon);

    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), fileNestButton);
    gtk_widget_set_halign(fileNestButton, GTK_ALIGN_START);
    gtk_widget_set_valign(fileNestButton, GTK_ALIGN_START);
    gtk_widget_set_margin_start(fileNestButton, 30);
    gtk_widget_set_margin_top(fileNestButton, 530); 

    g_signal_connect(fileNestButton, "button-press-event", G_CALLBACK(on_fileNest_icon_click), NULL);
}

void add_music_player_icon() {
    GError* error = NULL;
    GdkPixbuf* pixbuf = gdk_pixbuf_new_from_file("audio.png", &error);
    if (!pixbuf) {
        g_printerr("Failed to load image: %s\n", error->message);
        g_error_free(error);
        return;
    }

    GdkPixbuf* scaled_icon = gdk_pixbuf_scale_simple(pixbuf, 64, 64, GDK_INTERP_BILINEAR);
    GtkWidget* audioIcon = gtk_image_new_from_pixbuf(scaled_icon);
    g_object_unref(pixbuf);

    GtkWidget* audioButton = gtk_event_box_new();
    gtk_container_add(GTK_CONTAINER(audioButton), audioIcon);

    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), audioButton);
    gtk_widget_set_halign(audioButton, GTK_ALIGN_START);
    gtk_widget_set_valign(audioButton, GTK_ALIGN_START);
    gtk_widget_set_margin_start(audioButton, 30);
    gtk_widget_set_margin_top(audioButton, 630);

    g_signal_connect(audioButton, "button-press-event", G_CALLBACK(on_audio_icon_click), NULL);
}

void add_video_player_icon() {
    GError* error = NULL;
    GdkPixbuf* pixbuf = gdk_pixbuf_new_from_file("video.png", &error);
    if (!pixbuf) {
        g_printerr("Failed to load image: %s\n", error->message);
        g_error_free(error);
        return;
    }

    GdkPixbuf* scaled_icon = gdk_pixbuf_scale_simple(pixbuf, 64, 64, GDK_INTERP_BILINEAR);
    GtkWidget* videoIcon = gtk_image_new_from_pixbuf(scaled_icon);
    g_object_unref(pixbuf);

    GtkWidget* videoButton = gtk_event_box_new();
    gtk_container_add(GTK_CONTAINER(videoButton), videoIcon);

    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), videoButton);
    gtk_widget_set_halign(videoButton, GTK_ALIGN_START);
    gtk_widget_set_valign(videoButton, GTK_ALIGN_START);
    gtk_widget_set_margin_start(videoButton, 30);
    gtk_widget_set_margin_top(videoButton, 730);

    g_signal_connect(videoButton, "button-press-event", G_CALLBACK(on_video_icon_click), NULL);
}

void add_task_player_icon() {
    GError* error = NULL;
    GdkPixbuf* pixbuf = gdk_pixbuf_new_from_file("task.png", &error);
    if (!pixbuf) {
        g_printerr("Failed to load image: %s\n", error->message);
        g_error_free(error);
        return;
    }

    GdkPixbuf* scaled_icon = gdk_pixbuf_scale_simple(pixbuf, 64, 64, GDK_INTERP_BILINEAR);
    GtkWidget* taskIcon = gtk_image_new_from_pixbuf(scaled_icon);
    g_object_unref(pixbuf);

    GtkWidget* taskButton = gtk_event_box_new();
    gtk_container_add(GTK_CONTAINER(taskButton), taskIcon);

    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), taskButton);
    gtk_widget_set_halign(taskButton, GTK_ALIGN_START);
    gtk_widget_set_valign(taskButton, GTK_ALIGN_START);
    gtk_widget_set_margin_start(taskButton, 140);
    gtk_widget_set_margin_top(taskButton, 30);

    g_signal_connect(taskButton, "button-press-event", G_CALLBACK(on_task_icon_click), NULL);
}

void add_home_toggle_icon() {
    GError* error = NULL;
    GdkPixbuf* pixbuf = gdk_pixbuf_new_from_file("image.png", &error);
    if (!pixbuf) {
        g_printerr("Failed to load image: %s\n", error->message);
        g_error_free(error);
        return;
    }

    GdkPixbuf* scaled_icon = gdk_pixbuf_scale_simple(pixbuf, 64, 64, GDK_INTERP_BILINEAR);
    image = gtk_image_new_from_pixbuf(scaled_icon);
    g_object_unref(pixbuf);

    GtkWidget* homeButton = gtk_event_box_new();
    gtk_container_add(GTK_CONTAINER(homeButton), image);

    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), homeButton);
    gtk_widget_set_halign(homeButton, GTK_ALIGN_END);
    gtk_widget_set_valign(homeButton, GTK_ALIGN_START);
    gtk_widget_set_margin_end(homeButton, 30);
    gtk_widget_set_margin_top(homeButton, 30);

    g_signal_connect(homeButton, "button-press-event", G_CALLBACK(on_home_icon_click), NULL);
}

void add_shutDown_icon() {
    GError* error = NULL;
    GdkPixbuf* pixbuf = gdk_pixbuf_new_from_file("power.png", &error);
    if (!pixbuf) {
        g_printerr("Failed to load image: %s\n", error->message);
        g_error_free(error);
        return;
    }

    GdkPixbuf* scaled_icon = gdk_pixbuf_scale_simple(pixbuf, 64, 64, GDK_INTERP_BILINEAR);
    GtkWidget* powerIcon = gtk_image_new_from_pixbuf(scaled_icon);
    g_object_unref(pixbuf);

    GtkWidget* powerButton = gtk_event_box_new();
    gtk_container_add(GTK_CONTAINER(powerButton), powerIcon);

    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), powerButton);
    gtk_widget_set_halign(powerButton, GTK_ALIGN_START);
    gtk_widget_set_valign(powerButton, GTK_ALIGN_START);
    gtk_widget_set_margin_start(powerButton, 30);
    gtk_widget_set_margin_top(powerButton, 890);

    g_signal_connect(powerButton, "button-press-event", G_CALLBACK(on_power_icon_click), NULL);
}

