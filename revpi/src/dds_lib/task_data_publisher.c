#include "dds_dcps.h"
#include "CheckStatus.h"
#include "DDS_entities_manager.h"
#include "task_data.h"

void publish_task_data(const task_data_t* task_data) {

  DDS_Publisher message_Publisher;
  DDS_DataWriter message_DataWriter;
  printf("Creating a new DDS publisher for task data...\n");

  // Register the Topic's type in the DDS Domain.
  g_MessageTypeSupport = ListenerTaskData_TaskDataTypeSupport__alloc();
  checkHandle(g_MessageTypeSupport, "ListenerTaskData_TaskDataTypeSupport__alloc");
  registerMessageType(g_MessageTypeSupport);
  // Create the Topic's in the DDS Domain.
  g_MessageTypeName = ListenerTaskData_TaskDataTypeSupport_get_type_name(g_MessageTypeSupport);
  g_MessageTopic = createTopic("ListenerTaskData_TaskData", g_MessageTypeName);
  DDS_free(g_MessageTypeName);
  DDS_free(g_MessageTypeSupport);


  // Create the Publisher in the DDS Domain.
  message_Publisher = createPublisher();

  // Request a Reader from the the Subscriber.
  message_DataWriter = createDataWriter(message_Publisher, g_MessageTopic);

  // Initialize and pre-allocate the GuardList used to obtain the triggered Conditions.

  printf("Done creating Task Data Publisher...\n");

  printf("Publishing Data...\n");

  ListenerTaskData_TaskData *message = ListenerTaskData_TaskData__alloc();
  checkHandle(message, "ListenerTaskData_TaskData__alloc");
  message->taskID = task_data->task_ID;
  message->message = "A task ID";
  DDS_InstanceHandle_t message_handle = ListenerTaskData_TaskDataDataWriter_register_instance(message_DataWriter, message);

  DDS_ReturnCode_t status = ListenerTaskData_TaskDataDataWriter_write(message_DataWriter, message, message_handle);
  checkStatus(status, "ListenerTaskData_TaskDataDataWriter_write");

  printf("Finished sending data\n");

  // Dispose and unregister message
  status = ListenerTaskData_TaskDataDataWriter_dispose(message_DataWriter, message, message_handle);
  checkStatus(status, "ListenerTaskData_TaskDataDataWriter_dispose");
  status = ListenerTaskData_TaskDataDataWriter_unregister_instance(message_DataWriter, message, message_handle);
  checkStatus(status, "ListenerTaskData_TaskDataDataWriter_unregister_instance");

  DDS_free(message);

  deleteDataWriter(message_Publisher, message_DataWriter);

  deletePublisher(message_Publisher);

  deleteTopic(g_MessageTopic);

  DDS_free(g_MessageTypeName);
  DDS_free(g_MessageTypeSupport);

}