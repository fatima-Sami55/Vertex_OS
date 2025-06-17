#define NOMINMAX

#include <windows.h>
#include <psapi.h>
#include <gtk/gtk.h>
#include <sstream>
#include <vector>
#include <algorithm>
#include "globals.h" // Assuming this defines AppInfo and extern std::vector<AppInfo> runningApps

// Global vectors to store history data
std::vector<double> ram_history(100, 0.0);
std::vector<double> disk_history(100, 0.0);

// Global GTK widgets
GtkWidget* ram_drawing_area;
GtkWidget* disk_drawing_area;
GtkWidget* app_list_vbox;

// Constants
const int MAX_HISTORY = 100;

// ------------------ Drawing Functions ------------------
gboolean draw_single_graph(GtkWidget* widget, cairo_t* cr, gpointer user_data) {
    std::vector<double>* history = static_cast<std::vector<double>*>(user_data);

    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);
    int width = allocation.width;
    int height = allocation.height;

    // Background
    cairo_set_source_rgb(cr, 0, 0, 0); // Black background
    cairo_paint(cr);

    // Grid Lines
    cairo_set_source_rgba(cr, 0, 0.5, 0, 0.5); // Dark green, semi-transparent
    cairo_set_line_width(cr, 1.0);
    int grid_spacing = 20;
    for (int x = 0; x < width; x += grid_spacing) {
        cairo_move_to(cr, x + 0.5, 0);
        cairo_line_to(cr, x + 0.5, height);
    }
    for (int y = 0; y < height; y += grid_spacing) {
        cairo_move_to(cr, 0, y + 0.5);
        cairo_line_to(cr, width, y + 0.5);
    }
    cairo_stroke(cr);

    // Graph Line
    cairo_set_line_width(cr, 2.0);
    cairo_set_source_rgba(cr, 0, 1, 0, 0.9); // Bright green

    if (history->size() < 2) return FALSE; // <- FIXED: Avoid flat lines

    double step = (double)width / (history->size() - 1);

    auto transform = [height](double val) -> double {
        double transformed_val = val * 1.2 - 0.1;
        transformed_val = std::max(0.0, std::min(1.0, transformed_val));
        return height - (transformed_val * height * 0.9 + height * 0.05);
        };

    cairo_move_to(cr, 0, transform(history->front()));
    for (size_t i = 1; i < history->size(); ++i) {
        double x = i * step;
        double y = transform(history->at(i));
        cairo_line_to(cr, x, y);
    }
    cairo_stroke(cr);

    // Center Line
    cairo_set_source_rgba(cr, 1, 0, 0, 0.5);
    cairo_move_to(cr, width / 2.0, 0);
    cairo_line_to(cr, width / 2.0, height);
    cairo_stroke(cr);

    return FALSE;
}

// ------------------ Graph Data Update ------------------
gboolean update_graph_data(gpointer user_data) {
    int total_ram = 0, total_disk = 0;
    // Ensure runningApps is accessible and updated here if it's not already
    // For this example, we'll assume it's populated elsewhere.
    // In a real application, you'd likely have a mechanism to fetch
    // current app data here or pass it in.

    // Simulate some changing data for demonstration
    static double ram_counter = 0.0;
    static double disk_counter = 0.0;

    // Generate some dynamic values for testing the graph movement
    // In your actual code, this would be based on real system data
    total_ram = (int)(sin(ram_counter) * 3000 + 4000); // Between 1000 and 7000
    total_disk = (int)(cos(disk_counter) * 100000 + 150000); // Between 50000 and 250000
    ram_counter += 0.1;
    disk_counter += 0.05;


    double ram_percent = std::min(1.0, static_cast<double>(total_ram) / 8000.0);
    double disk_percent = std::min(1.0, static_cast<double>(total_disk) / 256000.0);

    ram_history.push_back(ram_percent);
    disk_history.push_back(disk_percent);

    if (ram_history.size() > MAX_HISTORY) ram_history.erase(ram_history.begin());
    if (disk_history.size() > MAX_HISTORY) disk_history.erase(disk_history.begin());

    // Explicitly queue a redraw for both drawing areas
    if (GTK_IS_WIDGET(ram_drawing_area)) {
        gtk_widget_queue_draw(ram_drawing_area);
    }
    if (GTK_IS_WIDGET(disk_drawing_area)) {
        gtk_widget_queue_draw(disk_drawing_area);
    }

    return TRUE;
}

// ------------------ App List Update ------------------
gboolean update_apps_tab(gpointer user_data) {
    GList* children = gtk_container_get_children(GTK_CONTAINER(app_list_vbox));
    for (GList* iter = children; iter != NULL; iter = iter->next)
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    g_list_free(children);

    for (const auto& app : runningApps) {
        std::stringstream ss;
        ss << "App: " << app.name
            << " | RAM: " << app.ram_usage_mb << " MB"
            << " | Disk: " << app.disk_usage_mb << " MB";
        GtkWidget* label = gtk_label_new(ss.str().c_str());
        gtk_box_pack_start(GTK_BOX(app_list_vbox), label, FALSE, FALSE, 2);
    }

    gtk_widget_show_all(app_list_vbox);
    return TRUE;
}

// ------------------ Tab Creators ------------------
GtkWidget* create_apps_tab() {
    app_list_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    update_apps_tab(NULL);
    g_timeout_add(1000, update_apps_tab, NULL);
    return app_list_vbox;
}

GtkWidget* create_graph_tab() {
    GtkWidget* hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);

    // Disk Graph
    GtkWidget* disk_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    GtkWidget* disk_label = gtk_label_new("Disk Usage");
    gtk_box_pack_start(GTK_BOX(disk_vbox), disk_label, FALSE, FALSE, 0);

    disk_drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(disk_drawing_area, 400, 300);
    g_signal_connect(disk_drawing_area, "draw", G_CALLBACK(draw_single_graph), &disk_history);
    gtk_box_pack_start(GTK_BOX(disk_vbox), disk_drawing_area, TRUE, TRUE, 0);

    // RAM Graph
    GtkWidget* ram_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    GtkWidget* ram_label = gtk_label_new("RAM Usage");
    gtk_box_pack_start(GTK_BOX(ram_vbox), ram_label, FALSE, FALSE, 0);

    ram_drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(ram_drawing_area, 400, 300);
    g_signal_connect(ram_drawing_area, "draw", G_CALLBACK(draw_single_graph), &ram_history);
    gtk_box_pack_start(GTK_BOX(ram_vbox), ram_drawing_area, TRUE, TRUE, 0);

    gtk_box_pack_start(GTK_BOX(hbox), disk_vbox, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), ram_vbox, TRUE, TRUE, 0);

    g_timeout_add(1000, update_graph_data, NULL);
    return hbox;
}

// ------------------ Task Manager Entry ------------------
void launch_task() {
    GtkWidget* taskWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(taskWindow), "Task Manager");
    gtk_window_set_default_size(GTK_WINDOW(taskWindow), 1000, 600);
    gtk_window_set_position(GTK_WINDOW(taskWindow), GTK_WIN_POS_CENTER);

    GtkWidget* notebook = gtk_notebook_new();
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), create_apps_tab(), gtk_label_new("Running Apps"));
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), create_graph_tab(), gtk_label_new("Usage Graphs"));
    gtk_container_add(GTK_CONTAINER(taskWindow), notebook);

    // --- FIXES HERE ---
    update_graph_data(NULL);  // Force initial update
    gtk_widget_queue_draw(ram_drawing_area);
    gtk_widget_queue_draw(disk_drawing_area);

    g_signal_connect(taskWindow, "destroy", G_CALLBACK(+[](GtkWidget*, gpointer) {
        close_simulated_app("task");  // Replace with the relevant app name
        }), NULL);
    gtk_widget_show_all(taskWindow);
}
