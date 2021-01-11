#ifndef __TASKDATA_H__
#define __TASKDATA_H__

struct Task_Data {

    int task_ID;

};

void create_participant(const char*);

void listen_for_task_data(void (*callback)(struct Task_Data*));

void publish_task_data(struct Task_Data*);

#endif
