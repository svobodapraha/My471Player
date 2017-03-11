#ifndef WAITFORINCOMMINGFRAME_H
#define WAITFORINCOMMINGFRAME_H
#include <QDebug>

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/can.h>
#include <linux/can/raw.h>



typedef struct XMC_LMOCan
{
  uint32_t    can_identifier;
  uint8_t     can_data_length;
  uint8_t     can_data[8];
} XMC_LMOCan_t;

//function declaration, externs
void* fnWaitForIncommingFrame (void* unused);
extern int iCanSocId;
extern pthread_mutex_t CanReadMutex;
#include "mainwindow.h"
#endif // WAITFORINCOMMINGFRAME_H
