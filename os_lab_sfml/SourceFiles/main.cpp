#include "ui.h" 
#include <gtk/gtk.h>
#include <gst/gst.h>

int main(int argc, char* argv[]) {
    gtk_init(&argc, &argv);
    gst_init(&argc, &argv);
    start_vertex_ui(); 
    gtk_main();
    return 0;
}
