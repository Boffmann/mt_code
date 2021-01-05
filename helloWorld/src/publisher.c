#include <stdlib.h>
#include <stdio.h>


#include "dds_dcps.h"
#include "CheckStatus.h"
#include "HelloWorld.h"


int main(int args, char** argv)
{
  // Generic entities
  DDS_DomainParticipantFactory  dpf;
  DDS_DomainParticipant         participant;
  DDS_Topic                     messageTopic;
  DDS_Publisher                 publisher;

  // QoS Policy
  DDS_TopicQos                  *reliable_topic_qos;
  DDS_PublisherQos              *pub_qos;

  // DDS Identifiers
  DDS_DomainId_t                domain = DDS_DOMAIN_ID_DEFAULT;
  DDS_InstanceHandle_t          message_handle;
  DDS_ReturnCode_t              status;

  // Type specific DDS entities
  HelloWorld_MessageTypeSupport messageTS;
  HelloWorld_MessageDataWriter  talker;

  // Sample definitions
  HelloWorld_Message            *msg;

  char                          *messageTypeName = NULL;
  char                          *partitionName = NULL;

  // Create DomainParticipantFactory and Domain Participant with default QoS settings
  dpf = DDS_DomainParticipantFactory_get_instance();
  checkHandle(dpf, "DDS_DomainParticipantFactory_get_instance");
  participant = DDS_DomainParticipantFactory_create_participant(
      dpf,                          // Factory to create domain participant
      domain,                       // Domain's ID
      DDS_PARTICIPANT_QOS_DEFAULT,  // Domain's QoS
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
  pub_qos = DDS_PublisherQos__alloc();
  checkHandle(pub_qos, "DDS_PublisherQos__alloc");
  status = DDS_DomainParticipant_get_default_publisher_qos(participant, pub_qos);
  checkStatus(status, "DDS_DomainParticipant_get_default_publisher_qos");
  pub_qos->partition.name._length = 1;
  pub_qos->partition.name._maximum = 1;
  pub_qos->partition.name._release = TRUE;
  pub_qos->partition.name._buffer = DDS_StringSeq_allocbuf(1);
  checkHandle(pub_qos->partition.name._buffer, "DDS_StringSeq_allocbuf");
  pub_qos->partition.name._buffer[0] = DDS_string_dup(partitionName);
  checkHandle(pub_qos->partition.name._buffer[0], "DDS_string_dup");

  // Create the publisher
  publisher = DDS_DomainParticipant_create_publisher(participant, pub_qos, NULL, DDS_STATUS_MASK_NONE);
  checkHandle(publisher, "DDS_DomainParticipant_create_publisher");

  // Create data writer for the topic
  talker = DDS_Publisher_create_datawriter(
      publisher,
      messageTopic,
      DDS_DATAWRITER_QOS_USE_TOPIC_QOS,
      NULL,
      DDS_STATUS_MASK_NONE);
  checkHandle(talker, "DDS_Publisher_create_datawriter");

  msg = HelloWorld_Message__alloc();
  checkHandle(msg, "HelloWorld_Message__alloc");
  msg->key = "Random Number";
  message_handle = HelloWorld_MessageDataWriter_register_instance(talker, msg);

  /* for (int i = 0; i < 120; ++i) { */
  while (1) {
    long value = rand() % 100;
    msg->value = value;
    printf("Writing Number: %ld\n", value);
    status = HelloWorld_MessageDataWriter_write(talker, msg, message_handle);
    checkStatus(status, "HelloWorld_MessageDataWriter_write");
    sleep(1);
  }

  // Dispose and unregister message
  status = HelloWorld_MessageDataWriter_dispose(talker, msg, message_handle);
  checkStatus(status, "HelloWorld_MessageDataWriter_dispose");
  status = HelloWorld_MessageDataWriter_unregister_instance(talker, msg, message_handle);
  checkStatus(status, "HelloWorld_MessageDataWriter_unregister_instance");

  DDS_free(msg);

  // Remove Data Writer
  status = DDS_Publisher_delete_datawriter(publisher, talker);
  checkStatus(status, "DDS_Publisher_delete_datawriter");

  // Remove publisher
  status = DDS_DomainParticipant_delete_publisher(participant, publisher);
  checkStatus(status, "DDS_DomainParticipant_delete_publisher");

  // Remove Topic
  status = DDS_DomainParticipant_delete_topic(participant, messageTopic);
  checkStatus(status, "DDS_DomainParticipant_delete_topic");

  // Deallocate QoS policies
  DDS_free(reliable_topic_qos);
  DDS_free(pub_qos);

  // De-allocate type names and TypeSupport
  DDS_free(messageTypeName);
  DDS_free(messageTS);

  // Remove Domain Participant
  status = DDS_DomainParticipantFactory_delete_participant(dpf, participant);
  checkStatus(status, "DDS_DomainParticipantFactory_delete_participant");

  printf("Completed example");
  return 0;

}

