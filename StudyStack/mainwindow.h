#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <QString>

QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    // Saves a new task to the database.
    void addTask();

    // Changes the stacked widget to the Home page.
    void showHomePage();

    // Changes the stacked widget to the Add Task page.
    void showAddTaskPage();

private:
    Ui::MainWindow *ui;

    // Connection to the SQLite database.
    QSqlDatabase database;

    // Opens the database and creates the tasks table.
    bool initializeDatabase();

    // Loads all saved tasks into the table.
    void loadTasks();

    // Adds one task to the table displayed on the Home page.
    void addTaskToTable(
        const QString &name,
        const QString &className,
        const QString &dueDate,
        double gradeWeight,
        double estimatedHours
        );
};

#endif // MAINWINDOW_H