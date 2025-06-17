#include <gtk/gtk.h>
#include <cstdlib>
#include <ctime>
#include <string>
#include "globals.h"

GtkWidget* dice_image;
const char* dice_images[6] = {
    "dice1.png", "dice2.png", "dice3.png",
    "dice4.png", "dice5.png", "dice6.png"
};

void on_roll_click(GtkWidget* widget, gpointer data) {
    int roll = rand() % 6; // Random number from 0 to 5
    gtk_image_set_from_file(GTK_IMAGE(dice_image), dice_images[roll]);
}

void launch_random() {
    gtk_init(NULL, NULL);
    srand(static_cast<unsigned int>(time(NULL))); // Seed once

    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "VERTEX Dice Roller");
    gtk_window_set_default_size(GTK_WINDOW(window), 250, 300);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    dice_image = gtk_image_new_from_file("dice1.png"); // Default face
    gtk_box_pack_start(GTK_BOX(vbox), dice_image, TRUE, TRUE, 10);

    GtkWidget* roll_button = gtk_button_new_with_label("Roll Dice ");
    gtk_box_pack_start(GTK_BOX(vbox), roll_button, FALSE, FALSE, 5);
    g_signal_connect(roll_button, "clicked", G_CALLBACK(on_roll_click), NULL);

    g_signal_connect(window, "destroy", G_CALLBACK(+[](GtkWidget*, gpointer) {
        close_simulated_app("random_generator");
        gtk_main_quit();
        }), NULL);

    gtk_widget_show_all(window);
    gtk_main();
}
