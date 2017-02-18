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

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/can.h>
#include <linux/can/raw.h>

#include "waitforincommingframe.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

  int fnPlayFile(QString asFileToPlay);
  void onCanMessageReceived(int iCounter, XMC_LMOCan_t *ReceivedCanMsg);

public slots:
  void fnNewPlayRequest(int iInfo);

private slots:
    void on_btnReadFile_clicked();
    void on_btnPlay_clicked();
    void on_playProcessExit(int exitCode, QProcess::ExitStatus exitStatus);

private:
    Ui::MainWindow *ui;
    QSqlTableModel *SampleListModel;
    QTextStream    *SampleListStream;
    QProcess *playProcess;
    bool volatile boPlayInProcess;
    QList<uint64_t> ListToPlay;

signals:
    void fnSignalNewPlayRequest(int iInfo);

};

extern MainWindow *pMainWindow;

#endif // MAINWINDOW_H
