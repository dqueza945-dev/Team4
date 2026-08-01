#include <QtTest>              // Qt's testing tools
#include "../database.h"       // our Database class

// This class holds our task add/delete tests.
class TestTasks : public QObject
{
    Q_OBJECT

private slots:
    void testAddTaskSavesToDatabase();
    void testDeleteTaskRemovesFromDatabase();
};

void TestTasks::testAddTaskSavesToDatabase()
{
    Database db;
    db.init();

    bool addResult = db.addTaskForUser(1, "Test Task Add", "CS 370", "2026-08-01T10:00:00", 10.0, 2.0);
    int count = db.countTasksByName("Test Task Add");

    // the add should succeed, and the task should now exist exactly once
    QVERIFY(addResult == true);
    QVERIFY(count == 1);
}

void TestTasks::testDeleteTaskRemovesFromDatabase()
{
    Database db;
    db.init();

    // make sure a task exists first
    db.addTaskForUser(1, "Test Task Delete", "CS 370", "2026-08-01T10:00:00", 10.0, 2.0);

    bool deleteResult = db.deleteTaskByName("Test Task Delete");
    int count = db.countTasksByName("Test Task Delete");

    // the delete should succeed, and the task should no longer exist
    QVERIFY(deleteResult == true);
    QVERIFY(count == 0);
}

QTEST_MAIN(TestTasks)
#include "test_tasks.moc"