#include "database.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QDateTime>
#include <algorithm>

Database::Database() {}


bool Database::init() {

    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);

    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(dataDir + "/studystack.db");

    // try to open it. if it fails open, print reason
    if (!m_db.open()) {
        qDebug() << "Failed to open database:" << m_db.lastError().text();
        return false;
    }

    QSqlQuery query;

    // TABLE 1. users for the login
    query.exec("CREATE TABLE IF NOT EXISTS users ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "username TEXT UNIQUE NOT NULL, "
               "password_hash TEXT NOT NULL)");


    // TABLE 2. tasks one row per task, with all the fields from our proposal
    query.exec("CREATE TABLE IF NOT EXISTS tasks ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "user_id INTEGER NOT NULL, "
               "name TEXT NOT NULL, "
               "category TEXT, "
               "class_name TEXT, "
               "due_date TEXT, "
               "grade_weight REAL, "
               "estimated_hours REAL, "
               "status TEXT DEFAULT 'open', "
               "FOREIGN KEY(user_id) REFERENCES users(id))");

        // Success message
    qDebug() << "Database ready at:" << dataDir + "/studystack.db";
    return true;


}

int Database::checkLogin(const QString &username, const QString &password)
{
    QSqlQuery query;
    query.prepare("SELECT id FROM users WHERE username = :username AND password_hash = :password");
    query.bindValue(":username", username);
    query.bindValue(":password", password);

    // if the query fails or finds no match, the login is wrong
    if (!query.exec() || !query.next())
    {
        return -1;
    }

    // login is correct, return the users id
    return query.value(0).toInt();
}

bool Database::createAccount(const QString &username, const QString &password)
{
    QSqlQuery query;
    query.prepare("INSERT INTO users (username, password_hash) VALUES (:username, :password)");
    query.bindValue(":username", username);
    query.bindValue(":password", password);

    return query.exec();  // true if it worked, false if it failed
}

// Adds a task tied to a specific user. Returns true if the insert worked.
bool Database::addTaskForUser(int userId, const QString &name, const QString &className,
                              const QString &dueDate, double gradeWeight, double estimatedHours)
{
    QSqlQuery query;
    query.prepare(
        "INSERT INTO tasks (user_id, name, class_name, due_date, grade_weight, estimated_hours) "
        "VALUES (:user_id, :name, :class_name, :due_date, :grade_weight, :estimated_hours)"
        );
    query.bindValue(":user_id", userId);
    query.bindValue(":name", name);
    query.bindValue(":class_name", className);
    query.bindValue(":due_date", dueDate);
    query.bindValue(":grade_weight", gradeWeight);
    query.bindValue(":estimated_hours", estimatedHours);

    return query.exec();
}

// Counts how many tasks have this exact name. used to check if a task exists or not
int Database::countTasksByName(const QString &name)
{
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM tasks WHERE name = :name");
    query.bindValue(":name", name);
    query.exec();
    query.next();  // move to the first result row

    return query.value(0).toInt();  // the count
}

// Deletes a task by its name. Returns true if the delete ran successfully
bool Database::deleteTaskByName(const QString &name)
{
    QSqlQuery query;
    query.prepare("DELETE FROM tasks WHERE name = :name");
    query.bindValue(":name", name);

    return query.exec();
}

// Renames a task. This is used to confirm that editing a task actually updates the database.
bool Database::updateTaskName(const QString &oldName, const QString &newName)
{
    QSqlQuery query;
    query.prepare("UPDATE tasks SET name = :newName WHERE name = :oldName");
    query.bindValue(":newName", newName);
    query.bindValue(":oldName", oldName);

    return query.exec();
}

// Looks through all of a user's tasks and finds the one with the highest priority
QString Database::getSuggestedStartTask(int userId)
{
    QSqlQuery query;
    query.prepare(
        "SELECT name, due_date, estimated_hours, grade_weight FROM tasks WHERE user_id = :user_id"
        );
    query.bindValue(":user_id", userId);
    query.exec();

    QString bestTaskName = "";
    double bestPriorityScore = -1.0;

    while (query.next())
    {
        QString name = query.value(0).toString();
        QString dueDateString = query.value(1).toString();
        double estimatedHours = query.value(2).toDouble();
        double gradeWeight = query.value(3).toDouble();

        QDateTime dueDateTime = QDateTime::fromString(dueDateString, Qt::ISODate);
        qint64 secondsRemaining = QDateTime::currentDateTime().secsTo(dueDateTime);

        // skip tasks that are already overdue or have no time left
        if (secondsRemaining <= 0)
        {
            continue;
        }

        double hoursRemaining = secondsRemaining / 3600.0;

        // urgency: how tight is the timeline
        double urgency = estimatedHours / hoursRemaining;

        // priority score: urgency boosted by how much the grade weight matters
        double priorityScore = urgency * (1.0 + gradeWeight / 100.0);

        if (priorityScore > bestPriorityScore)
        {
            bestPriorityScore = priorityScore;
            bestTaskName = name;
        }
    }

    return bestTaskName;
}

// Returns every task, sorted from most urgent to least urgent using the priority formula
QStringList Database::getTasksSortedByPriority(int userId)
{
    QSqlQuery query;
    query.prepare(
        "SELECT name, due_date, estimated_hours, grade_weight FROM tasks WHERE user_id = :user_id"
        );
    query.bindValue(":user_id", userId);
    query.exec();

    // store each task alongside its priority score
    QList<QPair<double, QString>> scoredTasks;

    while (query.next())
    {
        QString name = query.value(0).toString();
        QString dueDateString = query.value(1).toString();
        double estimatedHours = query.value(2).toDouble();
        double gradeWeight = query.value(3).toDouble();

        QDateTime dueDateTime = QDateTime::fromString(dueDateString, Qt::ISODate);
        qint64 secondsRemaining = QDateTime::currentDateTime().secsTo(dueDateTime);

        double priorityScore;
        QString displayDate = dueDateTime.toString("MM/dd/yyyy hh:mm AP");

        if (secondsRemaining <= 0)
        {
            // overdue tasks always show at the very top
            priorityScore = 999999.0;
            displayDate += " (overdue)";
        }
        else
        {
            double hoursRemaining = secondsRemaining / 3600.0;
            double urgency = estimatedHours / hoursRemaining;
            priorityScore = urgency * (1.0 + gradeWeight / 100.0);
        }

        QString displayLine = name + " — " + displayDate;
        scoredTasks.append(qMakePair(priorityScore, displayLine));
    }

    // sort so highest priority score to come first
    std::sort(scoredTasks.begin(), scoredTasks.end(),
              [](const QPair<double, QString> &a, const QPair<double, QString> &b) {
                  return a.first > b.first;
              });

    QStringList result;
    for (const auto &pair : scoredTasks)
    {
        result.append(pair.second);
    }

    return result;
}