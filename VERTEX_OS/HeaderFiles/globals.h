// globals.h
#ifndef GLOBALS_H
#define GLOBALS_H

#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <vector>
#include <string>

extern GtkWidget* overlay;
extern GtkWidget* window;
extern GtkWidget* mainContainer;
extern GtkWidget* image;
extern GtkCssProvider* cssProvider;

extern GdkPixbuf* originalBackground;
extern GtkWidget* bgImageWidget;

struct SimulatedApp {
    std::string name;
    int ram_usage_mb;
    int disk_usage_mb;
};

extern std::vector<SimulatedApp> runningApps;


void close_simulated_app(const std::string& appName);
bool is_app_running(const std::string& appName);
void show_app_already_running_box(const gchar* appName);
#endif
