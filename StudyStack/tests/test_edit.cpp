#include <QtTest>              // Qt's testing tools
#include "../database.h"       // our Database class

// This class holds our edit/update test.
class TestEdit : public QObject
{
    Q_OBJECT

private slots:
    void testEditTaskUpdatesDatabase();
};

void TestEdit::testEditTaskUpdatesDatabase()
{
    Database db;
    db.init();

    // create a task to edit
    db.addTaskForUser(1, "Original Task Name", "CS 370", "2026-08-01T10:00:00", 10.0, 2.0);

    // rename it
    bool updateResult = db.updateTaskName("Original Task Name", "Updated Task Name");

    int oldNameCount = db.countTasksByName("Original Task Name");
    int newNameCount = db.countTasksByName("Updated Task Name");

    // the update should succeed, old name should be gone, new name should exist
    QVERIFY(updateResult == true);
    QVERIFY(oldNameCount == 0);
    QVERIFY(newNameCount == 1);
}

QTEST_MAIN(TestEdit)
#include "test_edit.moc"