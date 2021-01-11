#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "dds_dcps.h"
#include "CheckStatus.h"
#include "DDS_entities_manager.h"
#include "task_data_listener.h"
#include "task_data.h"

void create_dds_listener() {
  DDS_Subscriber message_Subscriber;
  DDS_DataReader message_DataReader;
  DDS_ConditionSeq* guardList = NULL;
  struct DDS_DataReaderListener *message_Listener;
  struct Listener_taskdata* Listener_data;
  DDS_StatusMask mask;
  printf("Creating a new DDS listener for task data...\n");

  // Register the Topic's type in the DDS Domain.
  g_MessageTypeSupport = ListenerTaskData_TaskDataTypeSupport__alloc();
  checkHandle(g_MessageTypeSupport, "ListenerTaskData_TaskDataTypeSupport__alloc");
  registerMessageType(g_MessageTypeSupport);
  // Create the Topic's in the DDS Domain.
  g_MessageTypeName = ListenerTaskData_TaskDataTypeSupport_get_type_name(g_MessageTypeSupport);
  g_MessageTopic = createTopic("ListenerTaskData_TaskData", g_MessageTypeName);
  DDS_free(g_MessageTypeName);
  DDS_free(g_MessageTypeSupport);

  // Create the Subscriber's in the DDS Domain.
  message_Subscriber = createSubscriber();

  // Request a Reader from the the Subscriber.
  message_DataReader = createDataReader(message_Subscriber, g_MessageTopic);

  /* Allocate the DataReaderListener interface. */
  message_Listener = DDS_DataReaderListener__alloc();
  checkHandle(message_Listener, "DDS_DataReaderListener__alloc");


  Listener_data = malloc(sizeof(struct Listener_taskdata));
  checkHandle(Listener_data, "malloc");
  Listener_data->message_DataReader = &message_DataReader;
  message_Listener->listener_data = Listener_data;
  message_Listener->on_data_available = on_task_data_available;


  mask =  DDS_DATA_AVAILABLE_STATUS | DDS_REQUESTED_DEADLINE_MISSED_STATUS;
  g_status = DDS_DataReader_set_listener(message_DataReader, message_Listener, mask);
  checkStatus(g_status, "DDS_DataReader_set_listener");

  // WaitSet is used to avoid spinning in the loop below.

  // Initialize and pre-allocate the GuardList used to obtain the triggered Conditions.

  guardList = DDS_ConditionSeq__alloc();
  checkHandle(guardList, "DDS_ConditionSeq__alloc");
  guardList->_maximum = 1;
  guardList->_length = 0;
  guardList->_release = TRUE;
  guardList->_buffer = DDS_ConditionSeq_allocbuf(1);
  checkHandle(guardList->_buffer, "DDS_ConditionSeq_allocbuf");

  printf("Done creating Listener...\n");

}

void listen_for_task_data(void (*callback)(struct Task_Data* task_data)) {

  task_data_received_callback = callback;

  create_dds_listener();

}