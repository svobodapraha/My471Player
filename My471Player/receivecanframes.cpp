#include "receivecanframes.h"


ReceiveCanFrames_t::ReceiveCanFrames_t(QObject *parent) : QObject(parent)
{
  iThreadCounter = 0;
}

void ReceiveCanFrames_t::doWork()
{
  qDebug() << "QThread" << this;
  int recvbytes = 0;
  int iMessageCounter = -1;
  struct can_frame frame_rd;

  XMC_LMOCan_t ReceivedCanMsg;

  while(1)
  {
      if(iCanSocId < 0) break;
      recvbytes = read(iCanSocId, &frame_rd, sizeof(struct can_frame));
      if(recvbytes)
      {
        memset(&ReceivedCanMsg,0, sizeof(XMC_LMOCan_t));
        iMessageCounter++;
        ReceivedCanMsg.can_identifier  = frame_rd.can_id;
        ReceivedCanMsg.can_data_length = frame_rd.can_dlc;

        int u8SizeToCopy = 0;
        if (ReceivedCanMsg.can_data_length < (signed)sizeof(ReceivedCanMsg.can_data))
             u8SizeToCopy = ReceivedCanMsg.can_data_length;
         else
             u8SizeToCopy = sizeof(ReceivedCanMsg.can_data);
         memcpy(ReceivedCanMsg.can_data, frame_rd.data, u8SizeToCopy);

        qDebug() << "MsgInThread received: "<< ReceivedCanMsg.can_identifier << iMessageCounter;
        //Filter message for dhs
        if(ReceivedCanMsg.can_identifier == TO_DHS_Id)
        {
           uint16_t u16DhsCmd = *((uint16_t*)(((char*)(&ReceivedCanMsg.can_data))+0));
           if(u16DhsCmd == DHS_CMD_SAMPLE)
           {
             uint16_t u16DhsSampleNo = *((uint16_t*)(((char*)(&ReceivedCanMsg.can_data))+4));
             qDebug() << "PLAY SAMPLE " << u16DhsSampleNo;
             MutexListSample.lock();
             if (ListSamplesToPlayFromCAN.count() < MAX_SAMPLES_IN_CAN_FIFO)
             {
               ListSamplesToPlayFromCAN << u16DhsSampleNo;
               fnSignalNewPlayRequestWhenCanRcv(1);
             }
             MutexListSample.unlock();
           }

        }

      }
  }
}



//cookie




