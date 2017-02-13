#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    DbManager SamplesList_db;

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
