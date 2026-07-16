#include "mainwindow.h"

#include "database.h"

#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);  // Start the Qt

    Database db;
    db.init();   // set up SQLite database

    MainWindow w;
    w.setWindowTitle("StudyStack");
    w.show();   // show the main window
    return QApplication::exec();

}
