#include "task_data_listener.h"
#include "CheckStatus.h"
#include "dds_dcps.h"
#include "listener_task_dataSacDcps.h"

task_data_received_t task_data_received_callback = NULL;

void on_task_data_available(void *Listener_data, DDS_DataReader reader) {
  //UNUSED(Listener_data);
  (void) Listener_data;
  DDS_ReturnCode_t status;
  DDS_sequence_ListenerTaskData_TaskData* message_seq = DDS_sequence_ListenerTaskData_TaskData__alloc();
  DDS_SampleInfoSeq* message_infoSeq = DDS_SampleInfoSeq__alloc();

  printf("Got a message:\n");


  status = ListenerTaskData_TaskDataDataReader_read(reader,
    message_seq,
    message_infoSeq,
    DDS_LENGTH_UNLIMITED,
    DDS_ANY_SAMPLE_STATE,
    DDS_NEW_VIEW_STATE,
    DDS_ANY_INSTANCE_STATE);
  checkStatus(status, "ListenerTaskData_TaskDataDataReader_read");

  long taskid = 0;

  if ( message_seq->_length > 0 ) {
    printf( "\n=== [ListenerListener::on_data_available] - message_seq->length : %d", message_seq->_length );
    if( message_infoSeq->_buffer[0].valid_data == TRUE ) {
      printf( "\n    --- message received ---" );
      printf( "\n    userID  : %d", message_seq->_buffer[0].taskID );
      printf( "\n    Message : \"%s\"", message_seq->_buffer[0].message );
      taskid = message_seq->_buffer[0].taskID;
    }
  }


  status = ListenerTaskData_TaskDataDataReader_return_loan(reader, message_seq, message_infoSeq);
  checkStatus(status, "ListenerTaskData_TaskDataDataReader_return_loan");


  if (taskid != 0) {
    printf("\n Calling callback...\n");
    struct Task_Data task_data;
    task_data.task_ID = taskid;
    task_data_received_callback(&task_data);
  }

}