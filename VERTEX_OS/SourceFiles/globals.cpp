#include "globals.h"
#include <algorithm>
#include <gtk/gtk.h>

void close_simulated_app(const std::string& appName) {
    runningApps.erase(
        std::remove_if(runningApps.begin(), runningApps.end(),
            [&appName](const SimulatedApp& app) {
                return app.name == appName;
            }),
        runningApps.end()
    );
}


bool is_app_running(const std::string& appName) {
    for (const auto& app : runningApps) {
        if (app.name == appName)
            return true;
    }
    return false;
}

void show_app_already_running_box(const gchar* appName) {
    GtkWidget* dialog = gtk_dialog_new_with_buttons(
        "App Already Running",
        NULL,
        GTK_DIALOG_MODAL,
        "_OK", GTK_RESPONSE_OK,
        NULL);

    GtkWidget* content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    // Beautiful label with some padding and styling
    gchar* message = g_strdup_printf("The app \"%s\" is already running.", appName);
    GtkWidget* label = gtk_label_new(message);
    gtk_widget_set_margin_top(label, 20);
    gtk_widget_set_margin_bottom(label, 20);
    gtk_widget_set_margin_start(label, 20);
    gtk_widget_set_margin_end(label, 20);

    PangoAttrList* attrs = pango_attr_list_new();
    pango_attr_list_insert(attrs, pango_attr_weight_new(PANGO_WEIGHT_BOLD));
    pango_attr_list_insert(attrs, pango_attr_scale_new(PANGO_SCALE_LARGE));
    gtk_label_set_attributes(GTK_LABEL(label), attrs);
    pango_attr_list_unref(attrs);

    gtk_box_pack_start(GTK_BOX(content_area), label, TRUE, TRUE, 0);

    gtk_widget_show_all(dialog);

    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);

    g_free(message);
}

