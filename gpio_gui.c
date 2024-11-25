#include <gtk/gtk.h>

#define ROWS 8
#define COLS 16

typedef struct {
    GtkWidget *button;
    gboolean state;
} Button;

Button buttons[ROWS][COLS];

void on_button_clicked(GtkWidget *widget, gpointer data) {
    Button *btn = (Button *)data;
    btn->state = !btn->state;
    if (btn->state) {
        gtk_widget_override_background_color(btn->button, GTK_STATE_FLAG_NORMAL, &(GdkRGBA){0, 1, 0, 1}); // Green
    } else {
        gtk_widget_override_background_color(btn->button, GTK_STATE_FLAG_NORMAL, &(GdkRGBA){1, 0, 0, 1}); // Red
    }
}

void create_buttons(GtkWidget *grid) {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            buttons[i][j].button = gtk_button_new_with_label("");
            buttons[i][j].state = FALSE;
            gtk_widget_set_size_request(buttons[i][j].button, 50, 50);
            gtk_grid_attach(GTK_GRID(grid), buttons[i][j].button, j, i, 1, 1);
            g_signal_connect(buttons[i][j].button, "clicked", G_CALLBACK(on_button_clicked), &buttons[i][j]);
            gtk_widget_override_background_color(buttons[i][j].button, GTK_STATE_FLAG_NORMAL, &(GdkRGBA){1, 0, 0, 1}); // Red
        }
    }
}

void run_gui() {
    GtkWidget *window;
    GtkWidget *grid;

    gtk_init(NULL, NULL);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Button Monitor");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 400);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);

    create_buttons(grid);

    gtk_widget_show_all(window);
    gtk_main();
}

