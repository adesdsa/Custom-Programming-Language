#include <QApplication>
#include <QIcon>

#include "includes/driver/ui/gui/main_window.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/icons/logo.png"));

    MainWindow window;
    window.show();

    return app.exec();
}
