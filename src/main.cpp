#include "global.h"
#include "MainWindow.h"

#include <gtkmm/application.h>

int main (int argc, char **argv)
{
    Glib::RefPtr<Gtk::Application> app = Gtk::Application::create(argc, argv, APP_PACKAGE);
    MainWindow mainWindow;

    return app->run(mainWindow);
}
