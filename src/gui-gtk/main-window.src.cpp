#include "main-window.src.hpp"

namespace gui
{

    MainWindow::MainWindow() {
        // Creates the GTK Window
        this->c_Window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_window_set_title(GTK_WINDOW(this->c_Window), "FSMTP Server GUI");
        gtk_window_set_default_size(GTK_WINDOW(this->c_Window), 700, 400);
    }

    void MainWindow::run() {
        // Displays the window
        gtk_widget_show_all(this->c_Window);
    }
};