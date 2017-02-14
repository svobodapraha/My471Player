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

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_btnReadFile_clicked();

    void on_btnPlay_clicked();

private:
    Ui::MainWindow *ui;
    QSqlTableModel *SampleListModel;
    QTextStream    *SampleListStream;

};

#endif // MAINWINDOW_H
