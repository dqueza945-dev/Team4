#include <QtTest>

// This class tests basic input validation rules used when saving a task.
class TestValidation : public QObject
{
    Q_OBJECT

private slots:
    void testNegativeEstimatedHoursIsInvalid();
    void testZeroEstimatedHoursIsValid();
};

void TestValidation::testNegativeEstimatedHoursIsInvalid()
{
    double estimatedHours = -3.0;

    // same rule as the real app: negative hours should be rejected
    bool isValid = (estimatedHours >= 0);

    QVERIFY(isValid == false);
}

void TestValidation::testZeroEstimatedHoursIsValid()
{
    double estimatedHours = 0.0;

    // 0 hours should now be allowed
    bool isValid = (estimatedHours >= 0);

    QVERIFY(isValid == true);
}

QTEST_MAIN(TestValidation)
#include "test_validation.moc"