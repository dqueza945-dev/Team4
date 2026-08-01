#include <QtTest>       // Qt's testing tools
#include "../database.h"    // our own Database class

// This class holds our tests.
class TestDatabase : public QObject
{
    Q_OBJECT

private slots:
    void testInitCreatesDatabase(); // Declaring our one test function
};

// The actual test
void TestDatabase::testInitCreatesDatabase()
{
    Database db;         // create a new Database object
    bool result = db.init();    // this is the function we're actually testing

    QVERIFY(result == true);    // check the result
}

// turns this file into a runnable test program
QTEST_MAIN(TestDatabase)
#include "test_database.moc"