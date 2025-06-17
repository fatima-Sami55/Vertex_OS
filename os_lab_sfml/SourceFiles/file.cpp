#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <windows.h>
#include "globals.h"

#define MAX_PATH_LENGTH 1024
#define MAX_FILE_CONTENT 4096

char selected_file_path[MAX_PATH_LENGTH] = "";

// Show message dialog
void show_message(const gchar* message) {
    GtkWidget* dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "%s", message);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

// File chooser dialog
void open_file_dialog(GtkWidget* widget, gpointer data) {
    GtkWidget* dialog = gtk_file_chooser_dialog_new("Select a file",
        NULL,
        GTK_FILE_CHOOSER_ACTION_OPEN,
        "_Cancel", GTK_RESPONSE_CANCEL,
        "_Open", GTK_RESPONSE_ACCEPT,
        NULL);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char* filepath = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        strncpy_s(selected_file_path, MAX_PATH_LENGTH, filepath, _TRUNCATE);
        g_free(filepath);
        show_message(selected_file_path);  // Optional: show path to confirm selection
    }
    gtk_widget_destroy(dialog);
}

// File delete
void delete_file(GtkWidget* widget, gpointer data) {
    if (strlen(selected_file_path) == 0) {
        show_message("Please select a file first.");
        return;
    }

    if (remove(selected_file_path) == 0)
        show_message("File deleted successfully.");
    else
        show_message("Failed to delete file.");
}

// File info
void show_file_info(GtkWidget* widget, gpointer data) {
    if (strlen(selected_file_path) == 0) {
        show_message("Please select a file first.");
        return;
    }

    struct _stat info;
    if (_stat(selected_file_path, &info) != 0) {
        show_message("Failed to get file info.");
        return;
    }

    gchar buffer[512];
    sprintf_s(buffer, sizeof(buffer),
        "Path: %s\nSize: %lld bytes\nModified: %lld\n",
        selected_file_path,
        (long long)info.st_size,
        (long long)info.st_mtime);
    show_message(buffer);
}

// View/print file content
void print_file_content(GtkWidget* widget, gpointer data) {
    if (strlen(selected_file_path) == 0) {
        show_message("Please select a file first.");
        return;
    }

    FILE* file;
    errno_t err = fopen_s(&file, selected_file_path, "r");
    if (err != 0 || file == NULL) {
        show_message("Failed to open file for reading.");
        return;
    }

    char buffer[MAX_FILE_CONTENT + 1] = { 0 };
    fread(buffer, 1, MAX_FILE_CONTENT, file);
    buffer[MAX_FILE_CONTENT] = '\0';  // Null-terminate
    fclose(file);

    // Show content
    GtkWidget* dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_OTHER, GTK_BUTTONS_OK, "File Content:\n\n%s", buffer);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

// Icon button
GtkWidget* create_icon_button(const char* filename) {
    GError* error = NULL;
    GdkPixbuf* pixbuf = gdk_pixbuf_new_from_file(filename, &error);
    if (!pixbuf) {
        g_printerr("Failed to load image %s: %s\n", filename, error->message);
        g_error_free(error);
        return gtk_button_new_with_label("Error");
    }

    GdkPixbuf* scaled_pixbuf = gdk_pixbuf_scale_simple(pixbuf, 64, 64, GDK_INTERP_BILINEAR);
    GtkWidget* image = gtk_image_new_from_pixbuf(scaled_pixbuf);
    GtkWidget* button = gtk_button_new();
    gtk_button_set_image(GTK_BUTTON(button), image);

    g_object_unref(pixbuf);
    g_object_unref(scaled_pixbuf);
    return button;
}

// Main UI
void launch_FileNest() {
    gtk_init(NULL, NULL);

    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "VERTEX FileNest");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);

    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    GtkWidget* browse_btn = gtk_button_new_with_label("Browse File");
    GtkWidget* delete_btn = create_icon_button("Del.png");
    GtkWidget* info_btn = create_icon_button("File_info.png");
    GtkWidget* print_btn = create_icon_button("printer.png");

    gtk_box_pack_start(GTK_BOX(vbox), browse_btn, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), delete_btn, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), info_btn, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(vbox), print_btn, FALSE, FALSE, 5);

    g_signal_connect(browse_btn, "clicked", G_CALLBACK(open_file_dialog), NULL);
    g_signal_connect(delete_btn, "clicked", G_CALLBACK(delete_file), NULL);
    g_signal_connect(info_btn, "clicked", G_CALLBACK(show_file_info), NULL);
    g_signal_connect(print_btn, "clicked", G_CALLBACK(print_file_content), NULL);
    g_signal_connect(window, "destroy", G_CALLBACK(+[](GtkWidget*, gpointer) {
        close_simulated_app("filenest");
        gtk_main_quit();
        }), NULL);

    gtk_widget_show_all(window);
    gtk_main();
}
