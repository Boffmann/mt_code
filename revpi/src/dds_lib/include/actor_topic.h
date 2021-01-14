#ifndef __RASPI_DDSLIB_ACTORTOPIC_H__
#define __RASPI_DDSLIB_ACTORTOPIC_H__

#include "dds_dcps.h"
#include "topic.h"
#include "domain_participant.h"


topic_t actors_topic_create(const domain_participant_t* domain_participant, const DDS_TopicQos* qos);


#endif