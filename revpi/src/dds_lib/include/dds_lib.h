#ifndef __DDSLIB_API_H__
#define __DDSLIB_API_H__

/*
 ******************************
 * PUBLIC API FOR THE DDS_LIB *
 ******************************
*/

#include "domain_participant.h"
#include "actor.h"
#include "topic.h"
#include "publisher.h"
#include "listener.h"






DDS_DataWriterQos* dw_qos_copy_from_topic_qos(const publisher_t* publisher, const topic_t* topic);

DDS_DataReaderQos* dr_qos_copy_from_topic_qos(const subscriber_t* subscriber, const topic_t* topic);

#endif