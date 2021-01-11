#include "task_data_listener.h"
#include "CheckStatus.h"
#include "dds_dcps.h"
#include "listener_task_dataSacDcps.h"

void on_task_data_available(void *Listener_data, DDS_DataReader reader) {
  struct Listener_taskdata    *pListener_taskdata = (struct Listener_taskdata*) Listener_data;
  DDS_ReturnCode_t status;
  DDS_sequence_ListenerTaskData_TaskData* message_seq = DDS_sequence_ListenerTaskData_TaskData__alloc();
  DDS_SampleInfoSeq* message_infoSeq = DDS_SampleInfoSeq__alloc();
  unsigned long i;

  printf("Got a message:\n");

  printf("data reader: %p", *pListener_taskdata->message_DataReader);

  status = ListenerTaskData_TaskDataDataReader_read(*pListener_taskdata->message_DataReader,
    message_seq,
    message_infoSeq,
    DDS_LENGTH_UNLIMITED,
    DDS_ANY_SAMPLE_STATE,
    DDS_NEW_VIEW_STATE,
    DDS_ANY_INSTANCE_STATE);
  checkStatus(status, "ListenerTaskData_TaskDataDataReader_read");

  if ( message_seq->_length > 0 ) {
    printf( "\n=== [ListenerListener::on_data_available] - message_seq->length : %d", message_seq->_length );
    i = 0;
    do {
      if( message_infoSeq->_buffer[i].valid_data == TRUE ) {
        printf( "\n    --- message received ---" );
        printf( "\n    userID  : %d", message_seq->_buffer[i].taskID );
        printf( "\n    Message : \"%s\"", message_seq->_buffer[i].message );
      }
    } while ( ++i < message_seq->_length );
  }

  status = ListenerTaskData_TaskDataDataReader_return_loan(*pListener_taskdata->message_DataReader, message_seq, message_infoSeq);
  checkStatus(status, "ListenerTaskData_TaskDataDataReader_return_loan");

  printf("Calling callback...\n");

  struct Task_Data task_data;
  task_data.task_ID = message_seq->_buffer[0].taskID;
  process_data_callback(&task_data);

}
