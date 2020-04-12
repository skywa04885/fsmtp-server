#pragma once

#include <gtk-3.0/gtk/gtk.h>

namespace gui
{
    class MainWindow
    {
    public:
        MainWindow();
        ~MainWindow();
    private:
        GtkWidget *c_Window;
    };
};