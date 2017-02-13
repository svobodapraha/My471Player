#ifndef DBMANAGER_H
#define DBMANAGER_H
#include <QSqlDatabase>
#include <QDebug>
#include <QSqlQuery>


class DbManager
{
public:
  DbManager();
public:
  QSqlDatabase m_db;
};

#endif // DBMANAGER_H
