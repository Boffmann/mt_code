#include "task_data.h"
#include "DDS_entities_manager.h"

void create_participant(const char* partition_name) {

  // Create DDS DomainParticipant
  createParticipant(partition_name);
  printf("1.1\n");

}