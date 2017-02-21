#ifndef RECEIVECANFRAMES_H
#define RECEIVECANFRAMES_H

#include <QObject>
#include <unistd.h>
#include <QDebug>
#include "waitforincommingframe.h"

class ReceiveCanFrames_t : public QObject
{
    Q_OBJECT
public:
    explicit ReceiveCanFrames_t(QObject *parent = 0);
    int iThreadCounter;

signals:
    void signalWorkFinished();
    void signalCanMessageReceived(int iMessageCounter);
public slots:
    void doWork();
};

#endif // RECEIVECANFRAMES_H
