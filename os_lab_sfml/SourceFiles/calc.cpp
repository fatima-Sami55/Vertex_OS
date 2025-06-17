#include <gtk/gtk.h>
#include <iostream>
#include <string>
#include <sstream>
#include "globals.h"


void on_calc_button_clicked(GtkWidget* button, gpointer data) {
    const char* label = gtk_button_get_label(GTK_BUTTON(button));
    std::string* input = static_cast<std::string*>(data);

    if (std::string(label) == "=") {
        double a, b;
        char op;
        std::stringstream ss(*input);
        if (ss >> a >> op >> b) {
            double result = 0;
            bool valid = true;
            switch (op) {
            case '+': result = a + b; break;
            case '-': result = a - b; break;
            case '*': result = a * b; break;
            case '/':
                if (b != 0) result = a / b;
                else valid = false;
                break;
            default: valid = false;
            }

            if (valid)
                *input = std::to_string(result);
            else
                *input = "Error";
        }
        else {
            *input = "Invalid Input";
        }
    }
    else if (std::string(label) == "C") {
        input->clear();
    }
    else {
        *input += label;
    }

    GtkWidget* display = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "display"));
    gtk_entry_set_text(GTK_ENTRY(display), input->c_str());
}

void launch_calculator() {
    gtk_init(NULL, NULL);

    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "VERTEX Calculator");
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 400);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    GtkWidget* display = gtk_entry_new();
    gtk_editable_set_editable(GTK_EDITABLE(display), FALSE);
    gtk_box_pack_start(GTK_BOX(vbox), display, FALSE, FALSE, 5);

    std::string* input = new std::string();

    const char* buttons[4][4] = {
        {"7", "8", "9", "/"},
        {"4", "5", "6", "*"},
        {"1", "2", "3", "-"},
        {"0", "C", "=", "+"}
    };

    for (int i = 0; i < 4; ++i) {
        GtkWidget* row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
        gtk_box_pack_start(GTK_BOX(vbox), row, TRUE, TRUE, 5);
        for (int j = 0; j < 4; ++j) {
            GtkWidget* btn = gtk_button_new_with_label(buttons[i][j]);
            gtk_widget_set_hexpand(btn, TRUE);
            gtk_widget_set_vexpand(btn, TRUE);

            g_object_set_data(G_OBJECT(btn), "display", display);
            g_signal_connect(btn, "clicked", G_CALLBACK(on_calc_button_clicked), input);

            gtk_box_pack_start(GTK_BOX(row), btn, TRUE, TRUE, 5);
        }
    }

    g_signal_connect(window, "destroy", G_CALLBACK(+[](GtkWidget*, gpointer) {
        close_simulated_app("calculator");
        gtk_main_quit();
        }), NULL);

    gtk_widget_show_all(window);
    gtk_main();

    delete input;
}
