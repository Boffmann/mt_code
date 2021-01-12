#ifndef __TASKDATA_H__
#define __TASKDATA_H__

typedef struct {

    int task_ID;

} task_data_t;

void create_participant(const char*);

void listen_for_task_data(void (*callback)(const task_data_t*));

void publish_task_data(const task_data_t*);

#endif
