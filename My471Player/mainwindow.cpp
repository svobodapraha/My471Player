#include "mainwindow.h"
#include "ui_mainwindow.h"

//can socket and thread global variables
pthread_t WaitForIncommingFrameId = -1;


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //******************************
    //* create and open can socket *
    //******************************
    //initialize and open can socket
    //open socket
    iCanSocId = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (iCanSocId < 0)
    {
      qDebug() << "Problem to open the socket for can interface";
    }

    //find index for selected device
    struct ifreq ifr;
#define USED_CAN_DEVICE "vcan0"
//#define USED_CAN_DEVICE "can0"
    strcpy(ifr.ifr_name, USED_CAN_DEVICE);  //virtual can
    //strcpy(ifr.ifr_name, "can0");   //real can
    if (ioctl(iCanSocId, SIOCGIFINDEX, &ifr) < 0)
    {
      qDebug() << "Problem to find index for can device " USED_CAN_DEVICE;
    }
    else
    {
      qDebug() << "Index for " USED_CAN_DEVICE" is" <<  ifr.ifr_ifindex;
    }

    //bind device
    struct sockaddr_can addr;
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    if(bind(iCanSocId, (struct sockaddr *) &addr, sizeof(addr)) < 0)
    {
      qDebug() << "Problem to bind " USED_CAN_DEVICE;
    }

    //************************************
    //* Create thread for reading frames *
    //************************************
    if ((pthread_create (&WaitForIncommingFrameId, NULL, &fnWaitForIncommingFrame, NULL)) == 0)
    {
       qDebug() << "Thread Created";
    }
    else
    {
      qDebug() << "Problem to Create Thread";
    }
    //*************************************
    //* set up database, models and views *
    //*************************************

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



    //***************************************
    //* set processes and other connections *
    //***************************************

    playProcess = new QProcess(this);
    connect(playProcess, SIGNAL(finished(int,QProcess::ExitStatus)),
            this, SLOT(on_playProcessExit(int,QProcess::ExitStatus)));

    connect(this, SIGNAL(fnSignalNewPlayRequest(int)),
            this, SLOT  (fnNewPlayRequest(int)));


}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_btnReadFile_clicked()
{
    bool boResult;
    QString lineString;
    QFile SampleListFile(QFileInfo(QCoreApplication::applicationDirPath()).filePath()+"/../samples/samples.txt");
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
   QStringList asFileListToPlay;
   bool boResult = false;
   iSampleNo = ui->plainTextEdit_SampleNo->toPlainText().toInt(&boResult);
   if(!boResult) iSampleNo = -1;
   qDebug() << iSampleNo;

   asFileListToPlay.clear();
   if(iSampleNo >= 0)
   {
     boResult = query.exec("select * from SAMPLES where SampleNo = "+QString::number(iSampleNo)+";");

     if(!boResult) qDebug() << "Problem to select samples";
     if(query.record().count() > 2)
     {
       while(query.next())
       {
         qDebug() << query.value(0).toInt();
         for (int i = 2; i < query.record().count(); ++i)
         {
           if(!query.value(i).isNull())
           {
             QString asFileToPlay;
             asFileToPlay = (query.value(i).toString());
             qDebug() << asFileToPlay;
             asFileToPlay = QFileInfo(QCoreApplication::applicationDirPath()).filePath()+"/../samples/"+asFileToPlay+".mp3";
             //asFileToPlay = "\""+asFileToPlay+"\"";
             asFileListToPlay << asFileToPlay;
           }
         }
       }
       if(!asFileListToPlay.isEmpty())
       {
         fnPlayFile(asFileListToPlay);
       }
     }
     else
     {

     }
   }

}

int MainWindow::fnPlayFile(QStringList &asFileListToPlay)
{

  QStringList asPlayerSwitches;
  asPlayerSwitches.clear();

  playProcess->setProcessChannelMode(QProcess::ForwardedChannels);
  playProcess->start("mplayer", asPlayerSwitches + asFileListToPlay);
  qDebug() << "Player Started";
  foreach (QString asItem, asFileListToPlay)
  {
    qDebug() << asItem;
  }
  boPlayInProcess = true;
  playProcess->waitForFinished(-1);
  return 0;
}

void  MainWindow::on_playProcessExit(int exitCode, QProcess::ExitStatus exitStatus)
{
  boPlayInProcess = false;
  qDebug() << "end" << exitCode << exitStatus;
}


void MainWindow::onCanMessageReceived(int iCounter, XMC_LMOCan_t *ReceivedCanMsg)
{
    static uint64_t u64LastValue = 0;
    uint64_t u64CurrentValue = 0;
    u64CurrentValue = *((uint64_t *)ReceivedCanMsg->can_data);
    qDebug() << "MessageCounter:"
             << iCounter
             << ReceivedCanMsg->can_identifier
             << ReceivedCanMsg->can_data_length
             << QByteArray((const char *)ReceivedCanMsg->can_data, ReceivedCanMsg->can_data_length).toHex()
             << "Diff:"
             << (u64CurrentValue-u64LastValue);
    u64LastValue = u64CurrentValue;

    //add to list...
    ListToPlay.append(u64CurrentValue);
    emit fnSignalNewPlayRequest(1);

}

void MainWindow::fnNewPlayRequest(int iInfo)
{
    iInfo = iInfo;
    foreach (int iSampleNo, ListToPlay)
    {
       qDebug() << "in list:" << iSampleNo;
    }
}
