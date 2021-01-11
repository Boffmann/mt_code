#ifndef __TASKDATALISTENER_H__
#define __TASKDATALISTENER_H__

#include "dds_dcps.h"
#include "CheckStatus.h"
#include "task_data.h"

typedef void (*task_data_received_t)(struct Task_Data*);

struct Listener_taskdata {
  DDS_DataReader* message_DataReader;
};

void on_task_data_available(void *Listener_data, DDS_DataReader reader);

extern task_data_received_t task_data_received_callback;


#endif