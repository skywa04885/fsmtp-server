#pragma once

#include <gtk-3.0/gtk/gtk.h>

namespace gui
{
    class MainWindow
    {
    public:
        MainWindow();

        void run();
    private:
        GtkWidget *c_Window;
    };
};