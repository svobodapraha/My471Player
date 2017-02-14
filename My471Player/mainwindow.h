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

};

#endif // MAINWINDOW_H
