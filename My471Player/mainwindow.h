#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "dbmanager.h"
#include <QSqlTableModel>
#include <QSqlError>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QSqlRecord>
#include <QProcess>
#include <QList>
#include <QThread>
#include <QMutex>

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/can.h>
#include <linux/can/raw.h>

#include "waitforincommingframe.h"
#include "receivecanframes.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

  int fnPlayFile(QStringList &asFileListToPlay);
  void onCanMessageReceived(int iCounter, XMC_LMOCan_t *ReceivedCanMsg);

public slots:


private slots:
    void fnReadSamplesDefFile();
    void fnPlayFromCanFIFOList(int iInfo);
    void on_playProcessExit(int exitCode, QProcess::ExitStatus exitStatus);

    void on_btnList_clicked();

    void on_btnPlay_clicked();

    void on_btnPlayFromFifo_clicked();

private:
    Ui::MainWindow *ui;
    QSqlTableModel *SampleListModel;
    QTextStream    *SampleListStream;
    QProcess *playProcess;
    bool volatile boPlayInProcess;

signals:
    void fnSignalNewPlayRequest(int iInfo);

};

extern MainWindow *pMainWindow;
extern QList<int> ListSamplesToPlayFromCAN;
extern QMutex MutexListSample;


#endif // MAINWINDOW_H
