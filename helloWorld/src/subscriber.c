#include <stdio.h>

#include "dds_dcps.h"
#include "CheckStatus.h"
#include "HelloWorld.h"

int main (int argc, char** argv) {

  //Generic entities
  DDS_DomainParticipantFactory    dpf;
  DDS_DomainParticipant           participant;
  DDS_Topic                       messageTopic;
  DDS_Subscriber                  subscriber;

  // Type-specific entities
  HelloWorld_MessageTypeSupport   messageTS;
  HelloWorld_MessageDataReader    reader;
  DDS_sequence_HelloWorld_Message *msgSeq;
  DDS_SampleInfoSeq               *infoSeq;

  // QosPolicy holders
  DDS_TopicQos                    *reliable_topic_qos;
  DDS_SubscriberQos               *sub_qos;

  //DDS Identifiers
  DDS_DomainId_t                 domain = DDS_DOMAIN_ID_DEFAULT;
  DDS_ReturnCode_t                status;

  char*                           partitionName;
  char *                          messageTypeName = NULL;

  // Create Domain Participant Factory and Participant
  dpf = DDS_DomainParticipantFactory_get_instance();
  checkHandle(dpf, "DDS_DomainParticipantFactory_get_instance");
  participant = DDS_DomainParticipantFactory_create_participant (
      dpf,
      domain,
      DDS_PARTICIPANT_QOS_DEFAULT,
      NULL,
      DDS_STATUS_MASK_NONE);
  checkHandle(participant, "DDS_DomainParticipantFactory_create_participant");

  // Register required datatype for message
  messageTS = HelloWorld_MessageTypeSupport__alloc();
  checkHandle(messageTS, "HelloWorld_MessageTypeSupport__alloc");
  messageTypeName = HelloWorld_MessageTypeSupport_get_type_name(messageTS);
  status = HelloWorld_MessageTypeSupport_register_type(
      messageTS,
      participant,
      messageTypeName);
  checkStatus(status, "HelloWorld_MessageTypeSupport_register_type");

  // Set the ReliabilityQosPolicy to Reliable
  reliable_topic_qos = DDS_TopicQos__alloc();
  checkHandle(reliable_topic_qos, "DDS_TopicQos__alloc");
  status = DDS_DomainParticipant_get_default_topic_qos(participant, reliable_topic_qos);
  checkStatus(status, "DDS_DomainParticipant_set_default_topic_qos");
  reliable_topic_qos->reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;

  // Make tailored QoS the new default
  status = DDS_DomainParticipant_set_default_topic_qos(participant, reliable_topic_qos);
  checkStatus(status, "DDS_DomainParticipant_set_default_topic_qos");

  // Define the message topic
  messageTopic = DDS_DomainParticipant_create_topic(
      participant,
      "HelloWorldMessage",
      messageTypeName,
      reliable_topic_qos,
      NULL,
      DDS_STATUS_MASK_NONE);
  checkHandle(messageTopic, "DDS_DomainParticipant_create_topic");

  // Adapt default PublisherQoS for writing messages into Shared Memory
  partitionName = "HelloWorldRoom";
  sub_qos = DDS_SubscriberQos__alloc();
  checkHandle(sub_qos, "DDS_SubscriberQos__alloc");
  status = DDS_DomainParticipant_get_default_subscriber_qos(participant, sub_qos);
  checkStatus(status, "DDS_DomainParticipant_get_default_subscriber_qos");
  sub_qos->partition.name._length = 1;
  sub_qos->partition.name._maximum = 1;
  sub_qos->partition.name._release = TRUE;
  sub_qos->partition.name._buffer = DDS_StringSeq_allocbuf(1);
  checkHandle(sub_qos->partition.name._buffer, "DDS_StringSeq_allocbuf");
  sub_qos->partition.name._buffer[0] = DDS_string_dup(partitionName);
  checkHandle(sub_qos->partition.name._buffer[0], "DDS_string_dup");

  // Create subscriber
  subscriber = DDS_DomainParticipant_create_subscriber(participant, sub_qos, NULL, DDS_STATUS_MASK_NONE);
  checkHandle(subscriber, "DDS_DomainParticipant_create_subscriber");

  // Create data reader
  reader = DDS_Subscriber_create_datareader(
      subscriber,
      messageTopic,
      DDS_DATAREADER_QOS_USE_TOPIC_QOS,
      NULL,
      DDS_STATUS_MASK_NONE);

  msgSeq = DDS_sequence_HelloWorld_Message__alloc();
  checkHandle(msgSeq, "DDS_sequence_HelloWorld_Message__alloc");
  infoSeq = DDS_SampleInfoSeq__alloc();
  checkHandle(infoSeq, "DDS_SampleInfoSeq__alloc");

  for (int i = 0; i < 10; ++i) {

    status = HelloWorld_MessageDataReader_take(
        reader,
        msgSeq,
        infoSeq,
        DDS_LENGTH_UNLIMITED,
        DDS_ANY_SAMPLE_STATE,
        DDS_ANY_VIEW_STATE,
        DDS_ALIVE_INSTANCE_STATE);
    checkStatus(status, "HelloWorld_MessageDataReader_take");

    for (DDS_unsigned_long i = 0; i < msgSeq->_length; i++) {
      HelloWorld_Message * msg = &(msgSeq->_buffer[i]);
      printf("Got a message: %d\n", msg->value);
      fflush(stdout);
    }

    status = HelloWorld_MessageDataReader_return_loan(reader, msgSeq, infoSeq);
    checkStatus(status, "HelloWorld_MessageDataReader_return_loan");

    sleep(1);

  }

  DDS_free(msgSeq);
  DDS_free(infoSeq);

  status = DDS_Subscriber_delete_datareader(subscriber, reader);
  checkStatus(status, "DDS_Subscriber_delete_datareader");

  status = DDS_DomainParticipant_delete_subscriber(participant, subscriber);
  checkStatus(status, "DDS_DomainParticipant_delete_subscriber");

  status = DDS_DomainParticipant_delete_topic(participant, messageTopic);
  checkStatus(status, "DDS_DomainParticipant_delete_topic");

  DDS_free(reliable_topic_qos);
  DDS_free(sub_qos);

  DDS_free(messageTypeName);
  DDS_free(messageTS);

  status = DDS_DomainParticipantFactory_delete_participant(dpf, participant);
  checkStatus(status, "DDS_DomainParticipantFactory_delete_participant");

  return 0;

}
