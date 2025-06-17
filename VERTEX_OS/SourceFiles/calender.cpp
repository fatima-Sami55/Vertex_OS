#include <gtk/gtk.h>
#include "globals.h"

// Callback when a date is selected
void on_date_selected(GtkCalendar* calendar, gpointer label) {
    guint year, month, day;
    gtk_calendar_get_date(calendar, &year, &month, &day);

    gchar* date_str = g_strdup_printf("Selected Date: %02d-%02d-%04d", day, month + 1, year);
    gtk_label_set_text(GTK_LABEL(label), date_str);
    g_free(date_str);
}

void launch_calendar() {
    gtk_init(NULL, NULL);

    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "VERTEX Calendar ");
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 300);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    GtkWidget* calendar = gtk_calendar_new();
    gtk_box_pack_start(GTK_BOX(vbox), calendar, TRUE, TRUE, 5);

    GtkWidget* label = gtk_label_new("Select a date...");
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 5);

    g_signal_connect(calendar, "day-selected", G_CALLBACK(on_date_selected), label);

    g_signal_connect(window, "destroy", G_CALLBACK(+[](GtkWidget*, gpointer) {
        close_simulated_app("calendar");
        gtk_main_quit();
        }), NULL);
    gtk_widget_show_all(window);
    gtk_main();
}
