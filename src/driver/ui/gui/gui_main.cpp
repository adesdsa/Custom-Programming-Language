#include <QApplication>

#include "includes/driver/ui/gui/main_window.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    MainWindow window;
    window.show();

    return app.exec();
}

