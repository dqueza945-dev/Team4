#include <QSqlQuery>
#include <QtTest>              // Qt's testing tools
#include "../database.h"       // our Database class

// This class holds our login tests.
class TestLogin : public QObject
{
    Q_OBJECT

private slots:
    void testLoginSucceedsWithCorrectPassword();
    void testLoginFailsWithWrongPassword();
};

void TestLogin::testLoginSucceedsWithCorrectPassword()
{
    Database db;
    db.init();

    // make sure a known account exists first
    QSqlQuery insertQuery;
    insertQuery.prepare("INSERT OR IGNORE INTO users (username, password_hash) VALUES (:u, :p)");
    insertQuery.bindValue(":u", "testuser1");
    insertQuery.bindValue(":p", "correctpassword");
    insertQuery.exec();

    int result = db.checkLogin("testuser1", "correctpassword");

    // a real user id should be 1, not -1
    QVERIFY(result != -1);
}

void TestLogin::testLoginFailsWithWrongPassword()
{
    Database db;
    db.init();

    int result = db.checkLogin("testuser1", "wrongpassword");

    // wrong password should return -1
    QVERIFY(result == -1);
}

QTEST_MAIN(TestLogin)
#include "test_login.moc"