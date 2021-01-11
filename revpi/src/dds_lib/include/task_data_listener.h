#ifndef __TASKDATALISTENER_H__
#define __TASKDATALISTENER_H__

#include "dds_dcps.h"
#include "CheckStatus.h"
#include "task_data.h"

struct Listener_taskdata {
  DDS_DataReader* message_DataReader;
};

void on_task_data_available(void *Listener_data, DDS_DataReader reader);

void (*process_data_callback)(struct Task_Data*);

#endif