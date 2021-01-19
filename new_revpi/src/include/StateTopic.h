#ifndef __STATETOPIC_H__
#define __STATETOPIC_H__

#include "dds_dcps.h"
#include "datamodel.h"

extern char* g_StateTypeName;
extern DDS_TypeSupport g_StateTypeSupport;
extern DDS_Topic g_StateTopic;

void stateTopic_create(const char* topicName);

void stateTopic_write(DDS_DataWriter dataWriter);

#endif