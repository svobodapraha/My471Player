#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    DbManager SamplesList_db;
    //inic
    SampleListModel  = NULL;
    SampleListStream = NULL;

    SampleListModel = new QSqlTableModel;
    SampleListModel->setTable("SAMPLES");
    qDebug() << SampleListModel->lastError().text();
    SampleListModel->select();
    qDebug() << SampleListModel->lastError().text();
    ui->tableViewSampleList->setModel(SampleListModel);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_btnReadFile_clicked()
{
    bool boResult;
    QString lineString;
    QFile SampleListFile(QFileInfo(QCoreApplication::applicationDirPath()).filePath()+"/samples.txt");
    boResult = SampleListFile.open(QIODevice::ReadOnly);
    if(!boResult) qDebug() << "File not opened";
    SampleListStream = new QTextStream(SampleListFile.readAll());

    while(1)
    {
      QSqlQuery query;
      QStringList lineStringList;
      lineString = SampleListStream->readLine();
      if(lineString == NULL) break;
      lineStringList = lineString.split(" ", QString::SkipEmptyParts);
      qDebug() << "newline:";

      int iLastId = -1;
      for (int i = 0; i < lineStringList.count(); ++i)
      {

        if (i==0)
        {
          query.prepare("insert into SAMPLES(SampleNo) values(:SampleNo);");
          QString asTemp = lineStringList[i];
          query.bindValue(":SampleNo", lineStringList.at(i).toInt());
          boResult = query.exec();
          if(!boResult) qDebug() << "problem to insert samples number";
          iLastId = query.lastInsertId().toInt();
        }
        else
        {
          QString asColumnName = QString("SampleFile%1").arg(i);
          boResult = query.exec("alter table SAMPLES add column "+ asColumnName + " TEXT NULL default NULL;");
          boResult = query.exec("update SAMPLES set "+asColumnName+" = \""+lineStringList.at(i)+"\" where id = "+QString::number(iLastId)+";");
          qDebug() << lineStringList[i];
        }
      }

    }
    SampleListModel->setTable("SAMPLES");
    SampleListModel->select();

}

void MainWindow::on_btnPlay_clicked()
{
   int iSampleNo = -1;
   QSqlQuery query;
   bool boResult = false;
   iSampleNo = ui->plainTextEdit_SampleNo->toPlainText().toInt(&boResult);
   if(!boResult) iSampleNo = -1;
   qDebug() << iSampleNo;
   if(iSampleNo >= 0)
   {
     boResult = query.exec("select * from SAMPLES where SampleNo = "+QString::number(iSampleNo)+";");

     if(!boResult) qDebug() << "Problem to select samples";
     //qDebug() << "col:" << query.record().count();
     if (query.record().count() > 2)
     if(1)
     {
       while(query.next())
       {
         qDebug() << query.value(0).toInt();
         for (int i = 2; i < query.record().count(); ++i)
         {
           if(!query.value(i).isNull())qDebug() << query.value(i).toString();
         }
       }
     }
     else
     {

     }
   }

}
