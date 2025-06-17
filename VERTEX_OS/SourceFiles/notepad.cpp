#include <gtk/gtk.h>
#include <fstream>
#include "globals.h"

GtkWidget* text_view;
GtkTextTag* bold_tag;
GtkTextTag* italic_tag;

void apply_tag(GtkTextTag* tag) {
    GtkTextBuffer* buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    GtkTextIter start, end;
    gtk_text_buffer_get_selection_bounds(buffer, &start, &end);
    gtk_text_buffer_apply_tag(buffer, tag, &start, &end);
}

void on_bold_clicked(GtkWidget* widget, gpointer data) {
    apply_tag(bold_tag);
}

void on_italic_clicked(GtkWidget* widget, gpointer data) {
    apply_tag(italic_tag);
}

void on_save_clicked(GtkWidget* widget, gpointer data) {
    GtkWidget* dialog = gtk_file_chooser_dialog_new("Save File",
        NULL,
        GTK_FILE_CHOOSER_ACTION_SAVE,
        "_Cancel", GTK_RESPONSE_CANCEL,
        "_Save", GTK_RESPONSE_ACCEPT,
        NULL);

    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

        GtkTextBuffer* buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
        GtkTextIter start, end;
        gtk_text_buffer_get_bounds(buffer, &start, &end);
        gchar* content = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);

        std::ofstream file(filename);
        if (file.is_open()) {
            file << content;
            file.close();
        }

        g_free(content);
        g_free(filename);
    }

    gtk_widget_destroy(dialog);
}

void launch_notepad() {
    gtk_init(NULL, NULL);

    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "VERTEX Notepad");
    gtk_window_set_default_size(GTK_WINDOW(window), 500, 400);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // Toolbar with buttons
    GtkWidget* toolbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, FALSE, 5);

    GtkWidget* bold_button = gtk_button_new_with_label("Bold");
    g_signal_connect(bold_button, "clicked", G_CALLBACK(on_bold_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(toolbar), bold_button, FALSE, FALSE, 0);

    GtkWidget* italic_button = gtk_button_new_with_label("Italic");
    g_signal_connect(italic_button, "clicked", G_CALLBACK(on_italic_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(toolbar), italic_button, FALSE, FALSE, 0);

    GtkWidget* save_button = gtk_button_new_with_label("Save As");
    g_signal_connect(save_button, "clicked", G_CALLBACK(on_save_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(toolbar), save_button, FALSE, FALSE, 0);

    // Scrolled Text View
    GtkWidget* scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);

    text_view = gtk_text_view_new();
    gtk_container_add(GTK_CONTAINER(scroll), text_view);

    // Setup formatting tags
    GtkTextBuffer* buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    GtkTextTagTable* tag_table = gtk_text_buffer_get_tag_table(buffer);

    bold_tag = gtk_text_tag_new("bold");
    g_object_set(bold_tag, "weight", PANGO_WEIGHT_BOLD, NULL);
    gtk_text_tag_table_add(tag_table, bold_tag);

    italic_tag = gtk_text_tag_new("italic");
    g_object_set(italic_tag, "style", PANGO_STYLE_ITALIC, NULL);
    gtk_text_tag_table_add(tag_table, italic_tag);

    g_signal_connect(window, "destroy", G_CALLBACK(+[](GtkWidget*, gpointer) {
        close_simulated_app("notepad");
        gtk_main_quit();
        }), NULL);

    gtk_widget_show_all(window);
    gtk_main();
}
