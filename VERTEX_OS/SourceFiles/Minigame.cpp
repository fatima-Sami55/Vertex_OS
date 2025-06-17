#include <gtk/gtk.h>
#include <string>
#include "globals.h"

GtkWidget* buttons[3][3];
gchar current_player = 'X';
GtkWidget* status_label;

void reset_game();

gboolean check_winner() {
    const gchar* b[3][3];
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            b[i][j] = gtk_button_get_label(GTK_BUTTON(buttons[i][j]));

    for (int i = 0; i < 3; i++) {
        if (b[i][0] && b[i][1] && b[i][2] &&
            g_strcmp0(b[i][0], b[i][1]) == 0 &&
            g_strcmp0(b[i][1], b[i][2]) == 0 &&
            g_strcmp0(b[i][0], "") != 0)
            return TRUE;
        if (b[0][i] && b[1][i] && b[2][i] &&
            g_strcmp0(b[0][i], b[1][i]) == 0 &&
            g_strcmp0(b[1][i], b[2][i]) == 0 &&
            g_strcmp0(b[0][i], "") != 0)
            return TRUE;
    }

    if (b[0][0] && b[1][1] && b[2][2] &&
        g_strcmp0(b[0][0], b[1][1]) == 0 &&
        g_strcmp0(b[1][1], b[2][2]) == 0 &&
        g_strcmp0(b[0][0], "") != 0)
        return TRUE;

    if (b[0][2] && b[1][1] && b[2][0] &&
        g_strcmp0(b[0][2], b[1][1]) == 0 &&
        g_strcmp0(b[1][1], b[2][0]) == 0 &&
        g_strcmp0(b[0][2], "") != 0)
        return TRUE;

    return FALSE;
}

gboolean board_full() {
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++) {
            const gchar* label = gtk_button_get_label(GTK_BUTTON(buttons[i][j]));
            if (label == NULL || g_strcmp0(label, "") == 0)
                return FALSE;
        }
    return TRUE;
}

void on_button_click(GtkWidget* widget, gpointer data) {
    const gchar* label = gtk_button_get_label(GTK_BUTTON(widget));
    if (label != NULL && g_strcmp0(label, "") != 0) return;

    gtk_button_set_label(GTK_BUTTON(widget), current_player == 'X' ? "X" : "O");
    gtk_widget_queue_draw(widget); // Force redraw so it shows

    if (check_winner()) {
        gchar* msg = g_strdup_printf("Player %c wins!", current_player);
        gtk_label_set_text(GTK_LABEL(status_label), msg);
        g_free(msg);
        return;
    }

    if (board_full()) {
        gtk_label_set_text(GTK_LABEL(status_label), "Draw! Click reset to play again.");
        return;
    }

    current_player = (current_player == 'X') ? 'O' : 'X';
    gchar* msg = g_strdup_printf("Player %c's turn", current_player);
    gtk_label_set_text(GTK_LABEL(status_label), msg);
    g_free(msg);
}

void on_reset_click(GtkWidget* widget, gpointer data) {
    reset_game();
}

void reset_game() {
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++) {
            gtk_button_set_label(GTK_BUTTON(buttons[i][j]), "");
            gtk_widget_queue_draw(buttons[i][j]); // Ensure it's visually cleared
        }

    current_player = 'X';
    gtk_label_set_text(GTK_LABEL(status_label), "Player X's turn");
}

void launch_Minigame() {
    gtk_init(NULL, NULL);

    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "VERTEX MiniGame - TicTacToe");
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 350);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    status_label = gtk_label_new("Player X's turn");
    gtk_box_pack_start(GTK_BOX(vbox), status_label, FALSE, FALSE, 5);

    GtkWidget* grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 5);
    gtk_box_pack_start(GTK_BOX(vbox), grid, TRUE, TRUE, 5);

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            buttons[i][j] = gtk_button_new();
            gtk_widget_set_size_request(buttons[i][j], 80, 80);
            gtk_grid_attach(GTK_GRID(grid), buttons[i][j], j, i, 1, 1);
            g_signal_connect(buttons[i][j], "clicked", G_CALLBACK(on_button_click), NULL);
        }
    }

    GtkWidget* reset_button = gtk_button_new_with_label("Reset");
    gtk_box_pack_start(GTK_BOX(vbox), reset_button, FALSE, FALSE, 5);
    g_signal_connect(reset_button, "clicked", G_CALLBACK(on_reset_click), NULL);

    g_signal_connect(window, "destroy", G_CALLBACK(+[](GtkWidget*, gpointer) {
        close_simulated_app("minigame");
        gtk_main_quit();
        }), NULL);

    gtk_widget_show_all(window);
    gtk_main();
}
