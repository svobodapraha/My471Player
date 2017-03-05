#include "mainwindow.h"
#include "ui_mainwindow.h"

//can socket and thread global variables
pthread_t WaitForIncommingFrameId = -1;
QList<int> ListSamplesToPlayFromCAN;
QMutex MutexListSample;


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ListSamplesToPlayFromCAN.clear();



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
    /*
    if ((pthread_create (&WaitForIncommingFrameId, NULL, &fnWaitForIncommingFrame, NULL)) == 0)
    {
       qDebug() << "Thread Created";
    }
    else
    {
      qDebug() << "Problem to Create Thread";
    }
    */

    //**************************************************
    //* Create thread for reading frames using QThread *
    //**************************************************
    QThread *ReceiveCanFramesThread = new QThread;
    ReceiveCanFrames_t *ReceiveCanFrames = new ReceiveCanFrames_t;
    ReceiveCanFrames->moveToThread(ReceiveCanFramesThread);

    connect(ReceiveCanFramesThread, SIGNAL(started()),            ReceiveCanFrames,       SLOT(doWork()));
    connect(ReceiveCanFrames,       SIGNAL(signalWorkFinished()), ReceiveCanFramesThread, SLOT(quit()));

    connect(ReceiveCanFramesThread, SIGNAL(finished()),           ReceiveCanFrames,       SLOT(deleteLater()));
    connect(ReceiveCanFramesThread, SIGNAL(finished()),           ReceiveCanFramesThread, SLOT(deleteLater()));

    //when frames is received:
    //connect(ReceiveCanFrames, SIGNAL(signalCanMessageReceived(int)), this,                SLOT(onCanMessageReceivedFromQThread(int)));
    ReceiveCanFramesThread->start();

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
    boPlayInProcess = false;

    connect(this, SIGNAL(fnSignalNewPlayRequest(int)),
            this, SLOT  (fnPlayFromCanFIFOList(int)), Qt::QueuedConnection);

    //*************************
    //* Read samples def file *
    //*************************
    fnReadSamplesDefFile();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::fnReadSamplesDefFile()
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






void MainWindow::fnPlayFromCanFIFOList(int iInfo)
{
   iInfo = iInfo;
   int iSampleNo = -1;
   bool boResult;
   QSqlQuery query;
   QStringList asFileListToPlay;

   //if Play is in process, wait for end.
//   if(boPlayInProcess)
//   {
//       qDebug() << "Play in process, waiting for end";
//       return;
//   }

   if (playProcess->state() != QProcess::NotRunning)
   {
       qDebug() << "Play in process, waiting for end " << playProcess->state();
       return;
   }




   //Check FIFO in ListSamplesToPlayFromCAN
   //There is a only samples number - sample or samples files to play is necessary to look up in database
   MutexListSample.lock();
   if (!ListSamplesToPlayFromCAN.isEmpty())
   {
     iSampleNo = ListSamplesToPlayFromCAN.at(0);
     ListSamplesToPlayFromCAN.removeAt(0);
     MutexListSample.unlock();
     qDebug() << "Sample from FIFO: "<< iSampleNo;
   }
   else
   {
     iSampleNo = -1;
     MutexListSample.unlock();
     return;
   }


   //loop up sampe/s file in database
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
       else
       {
         emit fnSignalNewPlayRequest(4);
         qDebug() << "No samples files assigned";
       }
     }
     else
     {
        emit fnSignalNewPlayRequest(3);
        qDebug() << "SampleNo not valid - negative";
     }
   }

}

int MainWindow::fnPlayFile(QStringList &asFileListToPlay)
{

  QStringList asPlayerSwitches;
  asPlayerSwitches.clear();

  //playProcess->setProcessChannelMode(QProcess::ForwardedChannels);
  playProcess->start("mplayer", asPlayerSwitches + asFileListToPlay);
  qDebug() << "Player Started";
  foreach (QString asItem, asFileListToPlay)
  {
    qDebug() << asItem;
  }
  boPlayInProcess = true;
  //playProcess->waitForFinished(-1);
  return 0;
}

//External process - player finished playing last sample
void  MainWindow::on_playProcessExit(int exitCode, QProcess::ExitStatus exitStatus)
{
  boPlayInProcess = false;
  qDebug() << "end" << exitCode << exitStatus;
  emit fnSignalNewPlayRequest(1);
}










//****************
//* GUI SLOTS    *
//****************

//Queue from user interface
void MainWindow::on_btnPlay_clicked()
{
    int iSampleNo = -1;
    bool boResult;
    iSampleNo = ui->plainTextEdit_SampleNo->toPlainText().toInt(&boResult);
    if(!boResult) iSampleNo = -1;
    qDebug() << iSampleNo;
    if (iSampleNo >= 0)
    {
      MutexListSample.lock();
      ListSamplesToPlayFromCAN << iSampleNo;
      MutexListSample.unlock();
    }

}

//check what is in list
void MainWindow::on_btnList_clicked()
{
    MutexListSample.lock();
    foreach (int iSampleNo, ListSamplesToPlayFromCAN)
    {
       qDebug() <<"In List: "<< iSampleNo;
    }
    MutexListSample.unlock();
}

//request to play
void MainWindow::on_btnPlayFromFifo_clicked()
{
    emit fnSignalNewPlayRequest(1);
}


//****************
//* NOT USED NOW *
//****************
//this method is called from thread created by pthread_create
//now we are using QThread
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
}



