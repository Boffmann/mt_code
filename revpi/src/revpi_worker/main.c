#include "dds_lib/include/dds_lib.h"
#include <stdio.h>
#include <unistd.h>


// int main(int argc, char *argv[]) {
int main() {

    domain_participant_t domain_participant = setup_dds_domain("Test_Partition");
    
    topic_t actors_topic = join_topic(&domain_participant, ACTORS); 

    publisher_t actors_publisher = add_publisher(&domain_participant);

    publisher_add_datawriter(&actors_publisher, &actors_topic);


    for (int i = 0; i < 10; ++i) {

        sleep(1);

    }

    publisher_cleanup(&actors_publisher, &domain_participant);

    topic_leave(&actors_topic, &domain_participant);
    
    domain_participant_delete(&domain_participant);

}