#ifndef FERO_H
#define FERO_H

#include <stdint.h>
#include <stdbool.h>

#define FERO_ALIGNMENT_SIZE 4
#define FERO_ALIGNED __attribute__((aligned(FERO_ALIGNMENT_SIZE)))
#define FERO_ALIGN(size) (((size + (FERO_ALIGNMENT_SIZE-1))/FERO_ALIGNMENT_SIZE)*FERO_ALIGNMENT_SIZE)

#define FERO_QUEUE_BUFFER_SIZE(capacity, item_size) (capacity * (FERO_ALIGN(item_size) + sizeof(uint32_t)))
#define FERO_QUEUE_BUFFER(name, capacity, item_size) uint8_t name[FERO_QUEUE_BUFFER_SIZE(capacity, item_size)] FERO_ALIGNED

#define FERO_SCHEDULER_BUFFER_SIZE(tasklet_capacity) (FERO_ALIGN(tasklet_capacity * sizeof(void*)))
#define FERO_SCHEDULER_BUFFER(name, tasklet_capacity) uint8_t name[FERO_SCHEDULER_BUFFER_SIZE(tasklet_capacity)] FERO_ALIGNED

#define FERO_NAME_SIZE 16

typedef uint64_t Fero_TimeNs;
typedef char Fero_Name[FERO_NAME_SIZE];
typedef Fero_TimeNs (*Fero_GetTime)(void);
typedef bool (*Fero_Tasklet)(void);

typedef enum 
{
    FERO_TASK_INVOCATION_NONE = 0,
    FERO_TASK_INVOCATION_ALWAYS = 1,
    FERO_TASK_INVOCATION_PERIODIC = 2,
    FERO_TASK_INVOCATION_QUEUE = 3,
} Fero_Task_InvocationType;

typedef struct {
    uint32_t itemSize;
    uint32_t capacity;
    uint32_t nextIndex;
    uint32_t lastIndex;
    uint8_t *backingBuffer;
    uint32_t count;
    uint32_t* sizes;
} Fero_Queue;

typedef struct {
    Fero_Name name;
    Fero_Task_InvocationType invocationType;
    Fero_Tasklet tasklet;
    Fero_Queue* queue;
    Fero_TimeNs period;
    Fero_TimeNs offset;
    Fero_TimeNs deadline;
    Fero_TimeNs nextActivationTime;
    uint32_t priority;
} Fero_Task;

typedef struct {
    uint32_t taskCapacity;
    uint32_t taskCount;
    uint8_t* buffer;
    Fero_Task** tasks;
} Fero_Scheduler;

/*---QUEUE---*/

bool Fero_Queue_init(
    Fero_Queue* self,
    const uint32_t capacity,
    const uint32_t itemSize,
    uint8_t* buffer
);

uint32_t Fero_Queue_getCount(
    Fero_Queue* self
);

bool Fero_Queue_put(
    Fero_Queue* self,
    uint8_t* buffer,
    const uint32_t size
);

bool Fero_Queue_get(
    Fero_Queue* self,
    uint8_t* buffer,
    uint32_t* size
);

/*---TASK---*/

bool Fero_Task_init(
    Fero_Task* self,
    Fero_Name name,
    Fero_Tasklet tasklet
);

bool Fero_Task_setAlwaysActive(
    Fero_Task* self
);

bool Fero_Task_setPeriodic(
    Fero_Task* self,
    const Fero_TimeNs period,
    const Fero_TimeNs offset
);

bool Fero_Task_setQueueActivated(
    Fero_Task* self,
    Fero_Queue* queue
);

bool Fero_Task_isDue(
    Fero_Task* self,
    const Fero_TimeNs time
);

bool Fero_Task_invoke(
    Fero_Task* self
);

/*---SCHEDULER---*/

bool Fero_Scheduler_init(
    Fero_Scheduler* self,
    const uint32_t taskCapacity,
    uint8_t* buffer
);

bool Fero_Scheduler_addTask(
    Fero_Scheduler* self, 
    Fero_Task* task,
    const uint32_t priority
);

bool Fero_Scheduler_invoke(
    Fero_Scheduler* self,
    const Fero_TimeNs time
);


#endif