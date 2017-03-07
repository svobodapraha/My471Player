#ifndef RECEIVECANFRAMES_H
#define RECEIVECANFRAMES_H


//defines
#define TO_DHS_Id        0x410U
#define DHS_CMD_VOLUME   0x652FU
#define DHS_CMD_ENABLE   0x662FU
#define DHS_CMD_PREPARE  0x6421U
#define DHS_CMD_SAMPLE   0x0100U
#define DMS_CMD_PLAY     0x001DU


#include <QObject>
#include <unistd.h>
#include <QDebug>
#include "waitforincommingframe.h"

#define MAX_SAMPLES_IN_CAN_FIFO 20

class ReceiveCanFrames_t : public QObject
{
    Q_OBJECT
public:
    explicit ReceiveCanFrames_t(QObject *parent = 0);
    int iThreadCounter;

signals:
    void signalWorkFinished();
    void signalCanMessageReceived(int iMessageCounter);
    void fnSignalNewPlayRequestWhenCanRcv(int iInfo);
public slots:
    void doWork();
};

#endif // RECEIVECANFRAMES_H
