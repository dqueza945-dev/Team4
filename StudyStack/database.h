#ifndef DATABASE_H
#define DATABASE_H

#include <QSqlDatabase>

class Database
{
public:
    // Updates an existing task's name. This is to test if edits change the data
    bool updateTaskName(const QString &oldName, const QString &newName);

    // Adds a task for a given user. Returns true if it saved successfully
    bool addTaskForUser(int userId, const QString &name, const QString &className,
                        const QString &dueDate, double gradeWeight, double estimatedHours);

    // Returns how many tasks exist with this exact name.
    int countTasksByName(const QString &name);

    // Deletes a task by name. Returns true if it worked.
    bool deleteTaskByName(const QString &name);

    // Creates a new user account. Returns true if successful, false if the username is taken or something failed
    bool createAccount(const QString &username, const QString &password);

    // Checks a username and password against the users table. Returns the users id if correct, -1 if not
    int checkLogin(const QString &username, const QString &password);

    Database();
    bool init();

private:
    QSqlDatabase m_db;
};

#endif // DATABASE_H
