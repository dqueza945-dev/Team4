#ifndef DATABASE_H
#define DATABASE_H

#include <QSqlDatabase>

class Database
{
public:
    Database();
    bool init();

private:
    QSqlDatabase m_db;
};

#endif // DATABASE_H
