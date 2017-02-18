#include "waitforincommingframe.h"

int iCanSocId = -1;
pthread_mutex_t CanReadMutex;

void* fnWaitForIncommingFrame (void* unused)
{
  //cookie
  unused=unused;
  qDebug() << "fnWaitForIncommingFrame started";
  struct can_frame frame_rd;
  int recvbytes = 0;
  int iMessageCounter = -1;
  XMC_LMOCan_t ReceivedCanMsg;
  memset(&ReceivedCanMsg,0, sizeof(XMC_LMOCan_t));

  while (1)
  {
      if(iCanSocId < 0) break;
      recvbytes = read(iCanSocId, &frame_rd, sizeof(struct can_frame));
      if(recvbytes)
      {
        memset(&ReceivedCanMsg,0, sizeof(XMC_LMOCan_t));
        iMessageCounter++;
        //fprintf(stderr,"in thread: dlc = %d, data = %s\n", frame_rd.can_dlc,frame_rd.data);
        ReceivedCanMsg.can_identifier  = frame_rd.can_id;
        ReceivedCanMsg.can_data_length = frame_rd.can_dlc;

        int u8SizeToCopy = 0;
        if (ReceivedCanMsg.can_data_length < (signed)sizeof(ReceivedCanMsg.can_data))
             u8SizeToCopy = ReceivedCanMsg.can_data_length;
         else
             u8SizeToCopy = sizeof(ReceivedCanMsg.can_data);
         memcpy(ReceivedCanMsg.can_data, frame_rd.data, u8SizeToCopy);

        pthread_mutex_lock(&CanReadMutex);
        qDebug() << "MsgInThread received: "<< ReceivedCanMsg.can_identifier << iMessageCounter;
        pMainWindow->onCanMessageReceived(iMessageCounter, &ReceivedCanMsg);
        pthread_mutex_unlock(&CanReadMutex);
      }
  }

  return NULL;
}
