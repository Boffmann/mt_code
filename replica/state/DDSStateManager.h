#ifndef __DDSSTATEMANAGER_H__
#define __DDSSTATEMANAGER_H__

#include "dds_dcps.h"
#include "DDSCreator/CheckStatus.h"
#include "DDSCreator/DDSEntitiesCreator.h"

DDS_Topic balise_Topic, baliseGroup_Topic, movementAuthority_Topic, trainState_Topic;
DDS_Subscriber balise_Subscriber, baliseGroup_Subscriber, movementAuthority_Subscriber, trainState_Subscriber;
DDS_DataReader balise_DataReader, baliseGroup_DataReader, movementAuthority_DataReader, trainState_DataReader;
DDS_Publisher balise_Publisher, baliseGroup_Publisher, movementAuthority_Publisher, trainState_Publisher;
DDS_DataWriter balise_DataWriter, baliseGroup_DataWriter, movementAuthority_DataWriter, trainState_DataWriter;

void DDSSetupState();
void DDSStateCleanup();

#endif