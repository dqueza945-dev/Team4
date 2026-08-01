#include <QtTest>              // Qt's testing tools
#include "../database.h"       // our Database class

// This class holds our signup tests.
class TestSignUp : public QObject
{
    Q_OBJECT

private slots:
    void testSignUpCreatesNewAccount();
    void testSignUpFailsWithDuplicateUsername();
};

void TestSignUp::testSignUpCreatesNewAccount()
{
    Database db;
    db.init();

    // use a unique username so this test can run more than once
    bool result = db.createAccount("newuser_test", "somepassword");

    QVERIFY(result == true);
}

void TestSignUp::testSignUpFailsWithDuplicateUsername()
{
    Database db;
    db.init();

    // create the account once
    db.createAccount("duplicate_test", "password1");

    // try to create it again with the same username
    bool result = db.createAccount("duplicate_test", "password2");

    // should fail because username is unique
    QVERIFY(result == false);
}

QTEST_MAIN(TestSignUp)
#include "test_signup.moc"