#ifndef __REVPI_WORKER_H__
#define __REVPI_WORKER_H__

#include "dds_lib/include/domain_participant.h"
#include "dds_lib/include/decision_topic.h"
#include "dds_lib/include/state_topic.h"
#include "dds_lib/include/task_topic.h"

extern volatile bool running;

void worker_main(domain_participant_t* domain_participant, topic_t* decision_topic, topic_t* state_topic, topic_t* task_topic);

#endif
