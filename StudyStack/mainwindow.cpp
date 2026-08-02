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
    ui->stackedWidget->setCurrentWidget(ui->LoginPage);

    // Make the table columns use the available width.
    ui->HomeTaskTableWidget
        ->horizontalHeader()
        ->setSectionResizeMode(QHeaderView::Interactive);

    ui->HomeTaskTableWidget->setColumnWidth(0, 150);  // Task Name
    ui->HomeTaskTableWidget->setColumnWidth(1, 100);  // Class
    ui->HomeTaskTableWidget->setColumnWidth(2, 280);  // Due Date (extra room for countdown)
    ui->HomeTaskTableWidget->setColumnWidth(3, 100);  // Hours

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

    // Delete the selected task when Delete Task is clicked.
    connect(
        ui->DeleteTaskButton,
        &QPushButton::clicked,
        this,
        &MainWindow::deleteTask
        );

    // Edit the selected task when Edit Task is clicked.
    connect(
        ui->EditTaskButton,
        &QPushButton::clicked,
        this,
        &MainWindow::editTask
        );
    // Attempt login when Login button is clicked.
    connect(
        ui->LoginButton,
        &QPushButton::clicked,
        this,
        &MainWindow::login
        );

    // Create account when Create Account button is clicked.
    connect(
        ui->CreateAccountButton,
        &QPushButton::clicked,
        this,
        &MainWindow::signUp
        );

    // Switch to Sign Up page.
    connect(
        ui->GoToSignUpButton,
        &QPushButton::clicked,
        this,
        &MainWindow::showSignUpPage
        );

    // Switch back to Login page.
    connect(
        ui->BackToLoginButton,
        &QPushButton::clicked,
        this,
        &MainWindow::showLoginPage
        );
    // Log out when Logout button is clicked.
    connect(
        ui->LogoutButton,
        &QPushButton::clicked,
        this,
        &MainWindow::logout
        );

    // database is already open in main.cpp
    // loadTasks(); will be called after successful login, not here
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showHomePage()
{
    loadTasks();
    updateSummaryStats();
    ui->stackedWidget->setCurrentWidget(ui->HomePage);
}

void MainWindow::showAddTaskPage()
{
    ui->stackedWidget->setCurrentWidget(ui->AddTaskPage);
    ui->AddTaskTitleLabel->setText("Add New Task!");


    ui->TaskNameLineEdit->setFocus();
}

void MainWindow::addTask()
{
    QString taskName =
        ui->TaskNameLineEdit->text().trimmed();

    QString className =
        ui->ClassLineEdit->text().trimmed();

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

        ui->ClassLineEdit->setFocus();
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

    if (gradeWeight < 0)
    {
        QMessageBox::warning(
            this,
            "Invalid Grade Weight",
            "Grade weight must be greater than 0 if this is a school assignment."
            );

        ui->GradeWeightSpinBox->setFocus();
        return;
    }

    if (estimatedHours < 0)
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

    QSqlQuery query;

    if (isEditingTask)
    {
        // Update the existing task instead of creating a new one.
        query.prepare(
            "UPDATE tasks SET "
            "name = :name, class_name = :class_name, due_date = :due_date, "
            "grade_weight = :grade_weight, estimated_hours = :estimated_hours "
            "WHERE name = :original_name"
            );
        query.bindValue(":original_name", editingTaskOriginalName);
    }
    else
    {
        // Insert a brand new task.
        query.prepare(
            "INSERT INTO tasks "
            "(user_id, name, class_name, due_date, "
            "grade_weight, estimated_hours) "
            "VALUES "
            "(:user_id, :name, :class_name, :due_date, "
            ":grade_weight, :estimated_hours)"
            );
        query.bindValue(":user_id", currentUserId);
    }

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

    bool wasEditing = isEditingTask;

    // Reset editing state now that the save succeeded.
    isEditingTask = false;
    editingTaskOriginalName.clear();

    if (wasEditing)
    {
        // Refresh the whole table so the updated values show correctly.
        loadTasks();
    }
    else
    {
        addTaskToTable(
            taskName,
            className,
            storedDueDate,
            gradeWeight,
            estimatedHours
            );
    }

    // Clear the form after saving.
    ui->TaskNameLineEdit->clear();
    ui->ClassLineEdit->clear();
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

void MainWindow::deleteTask()
{
    int row = ui->HomeTaskTableWidget->currentRow();

    if (row < 0)
    {
        QMessageBox::warning(
            this,
            "No Task Selected",
            "Please select a task to delete."
            );
        return;
    }

    QString taskName = ui->HomeTaskTableWidget->item(row, 0)->text();

    QSqlQuery query;
    query.prepare("DELETE FROM tasks WHERE name = :name");
    query.bindValue(":name", taskName);

    if (!query.exec())
    {
        QMessageBox::critical(
            this,
            "Delete Error",
            "The task could not be deleted.\n\n" + query.lastError().text()
            );
        return;
    }

    ui->HomeTaskTableWidget->removeRow(row);
}

void MainWindow::editTask()
{

    int row = ui->HomeTaskTableWidget->currentRow();

    if  (row < 0) {
        QMessageBox::warning(
            this,
            "No task Selected",
            "Please select a task to edit."
            );
        return;
    }

    // remember the task we are editing
    isEditingTask = true;
    editingTaskOriginalName = ui->HomeTaskTableWidget->item(row, 0)->text();

    // pre fill the add task form with this taasks current values
    ui->TaskNameLineEdit->setText(
        ui->HomeTaskTableWidget->item(row, 0)->text()
        );
    ui->ClassLineEdit->setText(
        ui->HomeTaskTableWidget->item(row, 1)->text()
        );

    // switch to the add task page to let the user make changes
    ui->stackedWidget->setCurrentWidget(ui->AddTaskPage);
    ui->AddTaskTitleLabel->setText("Edit Task");

}

void MainWindow::loadTasks()
{
    ui->HomeTaskTableWidget->setRowCount(0);

    QSqlQuery query;

    query.prepare(
        "SELECT name, class_name, due_date, "
        "grade_weight, estimated_hours "
        "FROM tasks "
        "WHERE user_id = :user_id "
        "ORDER BY due_date ASC"
        );
    query.bindValue(":user_id", currentUserId);

    if (!query.exec())
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

void MainWindow::updateSummaryStats() {
    // get every task's due date and hours for this user
    QSqlQuery query;
    query.prepare(
        "SELECT due_date, estimated_hours FROM tasks WHERE user_id = :user_id"
        );
    query.bindValue(":user_id", currentUserId);
    query.exec();

    int dueTodayCount = 0;
    double hoursPlannedToday = 0.0;
    int overdueCount = 0;

    QDate today = QDate::currentDate();

     // go through every task and sort it into today or overdue or neither
    while (query.next())
    {
        QString dueDateString = query.value(0).toString();
        double estimatedHours = query.value(1).toDouble();

        QDateTime dueDateTime = QDateTime::fromString(dueDateString, Qt::ISODate);
        QDate dueDate = dueDateTime.date();

        if (dueDate == today)
        {
            // task is due today. next count it and add its hours
            dueTodayCount++;
            hoursPlannedToday += estimatedHours;
        }
        else if (dueDateTime < QDateTime::currentDateTime())
        {
            // task's due date already passed
            overdueCount++;
        }
    }

    // update the three labels with the real numbers
    ui->TasksDueTodayLabel->setText(QString("Tasks Due Today: %1").arg(dueTodayCount));
    ui->HoursPlannedLabel->setText(QString("Hours Planned: %1").arg(hoursPlannedToday, 0, 'f', 1));
    ui->OverDueTasksLabel->setText(QString("OverDue Tasks: %1").arg(overdueCount));
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

    // calculate how much time is left until due date
    qint64 secondsRemaining = QDateTime::currentDateTime().secsTo(displayedDueDate);

    QString countdownText;

    if (secondsRemaining < 0) {
        countdownText = " (overdue)";
    } else {
        qint64 daysRemaining = secondsRemaining / 86400;
        qint64 hoursRemaining = (secondsRemaining % 86400) / 3600;

        if (daysRemaining > 0) {
            countdownText = QString(" (%1d %2h left)")
            .arg(daysRemaining)
                .arg(hoursRemaining);
        } else {
            qint64 minutesRemaining = (secondsRemaining % 3600) / 60;
            countdownText = QString(" (%1h %2m left)")
                                .arg(hoursRemaining)
                                .arg(minutesRemaining);
        }
    }


    // Check if there's less time remaining than the task needs.
    double hoursRemaining_total = secondsRemaining / 3600.0;
    bool isAtRisk = (secondsRemaining < 0) || (hoursRemaining_total < estimatedHours);

    QTableWidgetItem *dueDateItem = new QTableWidgetItem(
        displayedDueDate.toString("MM/dd/yyyy hh:mm AP") + countdownText
        );

    if (isAtRisk)
    {
        dueDateItem->setBackground(Qt::red);
        dueDateItem->setForeground(Qt::white);
    }

    ui->HomeTaskTableWidget->setItem(row, 2, dueDateItem);


    ui->HomeTaskTableWidget->setItem(
        row,
        3,
        new QTableWidgetItem(
            QString::number(
                estimatedHours,
                'f',
                2
                ) + " hours"
            )
        );
}
void MainWindow::showLoginPage()
{
    ui->stackedWidget->setCurrentWidget(ui->LoginPage);
}

void MainWindow::logout()
{
    currentUserId = -1;
    ui->HomeTaskTableWidget->setRowCount(0);
    showLoginPage();
}

void MainWindow::showSignUpPage()
{
    ui->stackedWidget->setCurrentWidget(ui->SignUpPage);
}

void MainWindow::login()
{
    QString username = ui->LoginUsernameLineEdit->text().trimmed();
    QString password = ui->LoginPasswordLineEdit->text();

    if (username.isEmpty() || password.isEmpty())
    {
        QMessageBox::warning(
            this,
            "Missing Info",
            "Please enter both a username and password."
            );
        return;
    }

    QSqlQuery query;
    query.prepare("SELECT id FROM users WHERE username = :username AND password_hash = :password");
    query.bindValue(":username", username);
    query.bindValue(":password", password);

    if (!query.exec() || !query.next())
    {
        QMessageBox::warning(
            this,
            "Login Failed",
            "Incorrect username or password."
            );
        return;
    }

    currentUserId = query.value(0).toInt();

    // Clear the login fields for next time.
    ui->LoginUsernameLineEdit->clear();
    ui->LoginPasswordLineEdit->clear();

    showHomePage();
}

void MainWindow::signUp()
{
    QString username = ui->SignUpUsernameLineEdit->text().trimmed();
    QString password = ui->SignUpPasswordLineEdit->text();

    if (username.isEmpty() || password.isEmpty())
    {
        QMessageBox::warning(
            this,
            "Missing Info",
            "Please enter both a username and password."
            );
        return;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO users (username, password_hash) VALUES (:username, :password)");
    query.bindValue(":username", username);
    query.bindValue(":password", password);

    if (!query.exec())
    {
        QMessageBox::critical(
            this,
            "Sign Up Error",
            "Could not create account. Username may already be taken.\n\n" + query.lastError().text()
            );
        return;
    }

    QMessageBox::information(
        this,
        "Account Created",
        "Account created! Please log in."
        );

    ui->SignUpUsernameLineEdit->clear();
    ui->SignUpPasswordLineEdit->clear();

    showLoginPage();
}