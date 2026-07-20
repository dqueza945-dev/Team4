#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QAbstractItemView>
#include <QCoreApplication>
#include <QDateTime>
#include <QHeaderView>
#include <QMessageBox>
#include <QPushButton>
#include <QSqlError>
#include <QSqlQuery>
#include <QTableWidgetItem>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Start the application on the Home page.
    ui->stackedWidget->setCurrentWidget(ui->HomePage);

    // Make the table columns use the available width.
    ui->HomeTaskTableWidget
        ->horizontalHeader()
        ->setSectionResizeMode(QHeaderView::Stretch);

    // Make users select an entire task row.
    ui->HomeTaskTableWidget->setSelectionBehavior(
        QAbstractItemView::SelectRows
        );

    // Prevent users from directly editing table cells.
    ui->HomeTaskTableWidget->setEditTriggers(
        QAbstractItemView::NoEditTriggers
        );

    // Start the due date at the current date and time.
    ui->dateTimeEdit->setDateTime(
        QDateTime::currentDateTime()
        );

    // Open the Add Task page from the Home page.
    connect(
        ui->OpenAddTaskButton,
        &QPushButton::clicked,
        this,
        &MainWindow::showAddTaskPage
        );

    // Return to the Home page from the Add Task page.
    connect(
        ui->BackToHomeScreen,
        &QPushButton::clicked,
        this,
        &MainWindow::showHomePage
        );

    // Save a task when the Add Task button is clicked.
    connect(
        ui->AddTaskButton,
        &QPushButton::clicked,
        this,
        &MainWindow::addTask
        );

    // Open the database and load saved tasks.
    if (initializeDatabase())
    {
        loadTasks();
    }
}

MainWindow::~MainWindow()
{
    if (database.isOpen())
    {
        database.close();
    }

    delete ui;
}

bool MainWindow::initializeDatabase()
{
    database = QSqlDatabase::addDatabase("QSQLITE");

    QString databasePath =
        QCoreApplication::applicationDirPath()
        + "/studystack.db";

    database.setDatabaseName(databasePath);

    if (!database.open())
    {
        QMessageBox::critical(
            this,
            "Database Error",
            "The database could not be opened.\n\n"
                + database.lastError().text()
            );

        return false;
    }

    QSqlQuery query(database);

    QString createTableCommand =
        "CREATE TABLE IF NOT EXISTS tasks ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "name TEXT NOT NULL, "
        "class_name TEXT NOT NULL, "
        "due_date TEXT NOT NULL, "
        "grade_weight REAL NOT NULL, "
        "estimated_hours REAL NOT NULL"
        ")";

    if (!query.exec(createTableCommand))
    {
        QMessageBox::critical(
            this,
            "Database Error",
            "The tasks table could not be created.\n\n"
                + query.lastError().text()
            );

        return false;
    }

    return true;
}

void MainWindow::showHomePage()
{
    if (database.isOpen())
    {
        loadTasks();
    }

    ui->stackedWidget->setCurrentWidget(ui->HomePage);
}

void MainWindow::showAddTaskPage()
{
    ui->stackedWidget->setCurrentWidget(ui->AddTaskPage);

    ui->TaskNameLineEdit->setFocus();
}

void MainWindow::addTask()
{
    QString taskName =
        ui->TaskNameLineEdit->text().trimmed();

    QString className =
        ui->ClassComboBox->currentText().trimmed();

    QDateTime dueDateTime =
        ui->dateTimeEdit->dateTime();

    double gradeWeight =
        ui->GradeWeightSpinBox->value();

    double estimatedHours =
        ui->EstimatedHoursSpinBox->value();

    if (taskName.isEmpty())
    {
        QMessageBox::warning(
            this,
            "Missing Task Name",
            "Please enter a task name."
            );

        ui->TaskNameLineEdit->setFocus();
        return;
    }

    if (className.isEmpty())
    {
        QMessageBox::warning(
            this,
            "Missing Class",
            "Please select or enter a class."
            );

        ui->ClassComboBox->setFocus();
        return;
    }

    if (dueDateTime < QDateTime::currentDateTime())
    {
        QMessageBox::warning(
            this,
            "Invalid Due Date",
            "Please select a future due date."
            );

        ui->dateTimeEdit->setFocus();
        return;
    }

    if (gradeWeight <= 0)
    {
        QMessageBox::warning(
            this,
            "Invalid Grade Weight",
            "Grade weight must be greater than 0."
            );

        ui->GradeWeightSpinBox->setFocus();
        return;
    }

    if (estimatedHours <= 0)
    {
        QMessageBox::warning(
            this,
            "Invalid Estimated Hours",
            "Estimated hours must be greater than 0."
            );

        ui->EstimatedHoursSpinBox->setFocus();
        return;
    }

    QString storedDueDate =
        dueDateTime.toString(Qt::ISODate);

    QSqlQuery query(database);

    query.prepare(
        "INSERT INTO tasks "
        "(name, class_name, due_date, "
        "grade_weight, estimated_hours) "
        "VALUES "
        "(:name, :class_name, :due_date, "
        ":grade_weight, :estimated_hours)"
        );

    query.bindValue(":name", taskName);
    query.bindValue(":class_name", className);
    query.bindValue(":due_date", storedDueDate);
    query.bindValue(":grade_weight", gradeWeight);
    query.bindValue(":estimated_hours", estimatedHours);

    if (!query.exec())
    {
        QMessageBox::critical(
            this,
            "Save Error",
            "The task could not be saved.\n\n"
                + query.lastError().text()
            );

        return;
    }

    addTaskToTable(
        taskName,
        className,
        storedDueDate,
        gradeWeight,
        estimatedHours
        );

    // Clear the form after saving.
    ui->TaskNameLineEdit->clear();
    ui->ClassComboBox->setCurrentIndex(-1);
    ui->GradeWeightSpinBox->setValue(0);
    ui->EstimatedHoursSpinBox->setValue(0);

    ui->dateTimeEdit->setDateTime(
        QDateTime::currentDateTime()
        );

    QMessageBox::information(
        this,
        "Task Added",
        "The task was saved successfully."
        );

    // Return to the Home page after saving.
    showHomePage();
}

void MainWindow::loadTasks()
{
    ui->HomeTaskTableWidget->setRowCount(0);

    QSqlQuery query(database);

    if (!query.exec(
            "SELECT name, class_name, due_date, "
            "grade_weight, estimated_hours "
            "FROM tasks "
            "ORDER BY due_date ASC"
            ))
    {
        QMessageBox::critical(
            this,
            "Load Error",
            "The saved tasks could not be loaded.\n\n"
                + query.lastError().text()
            );

        return;
    }

    while (query.next())
    {
        QString taskName =
            query.value(0).toString();

        QString className =
            query.value(1).toString();

        QString dueDate =
            query.value(2).toString();

        double gradeWeight =
            query.value(3).toDouble();

        double estimatedHours =
            query.value(4).toDouble();

        addTaskToTable(
            taskName,
            className,
            dueDate,
            gradeWeight,
            estimatedHours
            );
    }
}

void MainWindow::addTaskToTable(
    const QString &name,
    const QString &className,
    const QString &dueDate,
    double gradeWeight,
    double estimatedHours
    )
{
    int row =
        ui->HomeTaskTableWidget->rowCount();

    ui->HomeTaskTableWidget->insertRow(row);

    QDateTime displayedDueDate =
        QDateTime::fromString(
            dueDate,
            Qt::ISODate
            );

    ui->HomeTaskTableWidget->setItem(
        row,
        0,
        new QTableWidgetItem(name)
        );

    ui->HomeTaskTableWidget->setItem(
        row,
        1,
        new QTableWidgetItem(className)
        );

    ui->HomeTaskTableWidget->setItem(
        row,
        2,
        new QTableWidgetItem(
            displayedDueDate.toString(
                "MM/dd/yyyy hh:mm AP"
                )
            )
        );

    ui->HomeTaskTableWidget->setItem(
        row,
        3,
        new QTableWidgetItem(
            QString::number(
                gradeWeight,
                'f',
                2
                ) + "%"
            )
        );

    ui->HomeTaskTableWidget->setItem(
        row,
        4,
        new QTableWidgetItem(
            QString::number(
                estimatedHours,
                'f',
                2
                ) + " hours"
            )
        );
}