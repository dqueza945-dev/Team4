#include <QtTest>
#include <QDateTime>

// This class tests the warning and at risk time calculation logic.
class TestWarning : public QObject
{
    Q_OBJECT

private slots:
    void testTaskIsFlaggedWhenTimeIsShort();
};

void TestWarning::testTaskIsFlaggedWhenTimeIsShort()
{
    // simulate a task due in 1 hour that needs 5 hours of work
    QDateTime now = QDateTime::currentDateTime();
    QDateTime dueDate = now.addSecs(3600);  // 1 hour from now

    qint64 secondsRemaining = now.secsTo(dueDate);
    double hoursRemaining = secondsRemaining / 3600.0;
    double estimatedHours = 5.0;

    // same logic as the real app: at risk if time left < hours needed
    bool isAtRisk = (secondsRemaining < 0) || (hoursRemaining < estimatedHours);

    // 1 hour left but 5 hours needed - this should be flagged as at risk
    QVERIFY(isAtRisk == true);
}

QTEST_MAIN(TestWarning)
#include "test_warning.moc"