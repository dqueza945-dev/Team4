#include "database.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

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