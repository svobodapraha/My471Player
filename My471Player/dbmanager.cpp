#include "dbmanager.h"

DbManager::DbManager()
{
  bool boResult = false;
  m_db = QSqlDatabase::addDatabase("QSQLITE");
  m_db.setDatabaseName(":memory:");
  if (m_db.open())
  {
    qDebug() << "db opened OK";
    QSqlQuery query;
    //create db structure
    boResult = query.exec(
                            "create table SAMPLES (id integer primary key autoincrement, "
                                                  "SampleNo integer);"

                         );

    if(!boResult) qDebug() << "problem to create table SAMPLES";

    boResult = query.exec("insert into SAMPLES values(1, 1)");
    if(!boResult) qDebug() << "problem to insert values SAMPLES";

  }
  else
  {
    qDebug() << "error to open db";
  }

}
