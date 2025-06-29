#include "fero.h"
#include <string.h>

/*---QUEUE---*/

bool Fero_Queue_init(
    Fero_Queue* self,
    const uint32_t capacity,
    const uint32_t itemSize,
    uint8_t* buffer
)
{
    if ((uintptr_t)buffer % FERO_ALIGNMENT_SIZE != 0)
    {
        // Alignment on 4 bytes is required
        return false;
    }
    self->itemSize = itemSize;
    self->capacity = capacity;
    self->backingBuffer = buffer;
    self->nextIndex = 0;
    self->lastIndex = 0;
    self->count = 0;
    self->sizes = (uint32_t*)&(buffer[FERO_ALIGN(self->itemSize * self->capacity)]);
    return true;
}

uint32_t Fero_Queue_getCount(
    Fero_Queue* self
)
{
    return self->count;
}

bool Fero_Queue_put(
    Fero_Queue* self,
    uint8_t* buffer,
    const uint32_t size
)
{
    if (size > self->itemSize)
    {
        return false;
    }
    if (self->count == self->capacity)
    {
        return false;
    }
    const uint32_t index = self->nextIndex;
    uint8_t* destination = &(self->backingBuffer[index * self->itemSize]);
    memcpy(destination, buffer, size);
    self->sizes[index] = size;
    self->nextIndex = (self->nextIndex + 1) % self->capacity;
    self->count++;
    return true;
}

bool Fero_Queue_get(
    Fero_Queue* self,
    uint8_t* buffer,
    uint32_t* size
)
{
    if (self->count == 0)
    {
        return false;
    }
    const uint32_t index = self->lastIndex;
    *size = self->sizes[index];
    uint8_t* source = &(self->backingBuffer[index * self->itemSize]);
    memcpy(buffer, source, *size);
    self->lastIndex = (self->lastIndex + 1) % self->capacity;
    self->count--;
    return true;
}

/*---TASK---*/

bool Fero_Task_init(
    Fero_Task* self,
    Fero_Name name,
    Fero_Tasklet tasklet
)
{
    memcpy(self->name, name, FERO_NAME_SIZE);
    self->tasklet = tasklet;
    self->invocationType = FERO_TASK_INVOCATION_NONE;
    self->queue = NULL;
    self->period = 0;
    self->offset = 0;
    self->deadline = 0;
    self->nextActivationTime = 0;
    self->priority = 0;
    return true;
}

bool Fero_Task_setAlwaysActive(
    Fero_Task* self
)
{
    self->invocationType = FERO_TASK_INVOCATION_ALWAYS;
    return true;
}

bool Fero_Task_setPeriodic(
    Fero_Task* self,
    const Fero_TimeNs period,
    const Fero_TimeNs offset
)
{
    self->invocationType = FERO_TASK_INVOCATION_PERIODIC;
    self->period = period;
    self->offset = offset;
    self->nextActivationTime = offset;
    return true;
}

bool Fero_Task_setQueueActivated(
    Fero_Task* self,
    Fero_Queue* queue
)
{
    self->invocationType = FERO_TASK_INVOCATION_QUEUE;
    self->queue = queue;
    return true;
}

bool Fero_Task_isDue(
    Fero_Task* self,
    const Fero_TimeNs time
)
{
    switch (self->invocationType)
    {
        case FERO_TASK_INVOCATION_ALWAYS:
            return true;
        case FERO_TASK_INVOCATION_PERIODIC:
            return self->nextActivationTime <= time;
        case FERO_TASK_INVOCATION_QUEUE:
            return Fero_Queue_getCount(self->queue) > 0;
        default:
            return false;
    }
}

bool Fero_Task_invoke(
    Fero_Task* self
)
{
    if (self->invocationType == FERO_TASK_INVOCATION_PERIODIC)
    {
        self->nextActivationTime += self->period;
    }
    self->tasklet();
    return true;
}

/*---SCHEDULER---*/

bool Fero_Scheduler_init(
    Fero_Scheduler* self,
    const uint32_t taskCapacity,
    uint8_t* buffer
)
{
    if ((uintptr_t)buffer % FERO_ALIGNMENT_SIZE != 0)
    {
        // Alignment on 4 bytes is required
        return false;
    }
    self->taskCapacity = taskCapacity;
    self->taskCount = 0;
    self->buffer = buffer;
    self->tasks = (Fero_Task**)buffer;
    return true;
}

bool Fero_Scheduler_addTask(
    Fero_Scheduler* self, 
    Fero_Task* task,
    const uint32_t priority
)
{
    if (self->taskCount == self->taskCapacity)
    {
        return false;
    }
    task->priority = priority;
    self->taskCount++;
    // Try to insert it before an existing one
    for (uint32_t index = 0; index < self->taskCount - 1; index++)
    {
        if (self->tasks[index]->priority < priority)
        {
            // Move all existing tasks by one place
            for (uint32_t backIndex = self->taskCount - 1; backIndex > index; backIndex--)
            {
                self->tasks[backIndex] = self->tasks[backIndex-1];
            }
            // Insert task
            self->tasks[index] = task;
            return true;
        }
    }
    // Was not inserted before an exising one, so add it at the end
    self->tasks[self->taskCount - 1] = task;
    return true;
}

bool Fero_Scheduler_invoke(
    Fero_Scheduler* self,
    const Fero_TimeNs time
)
{
    for (uint32_t index = 0; index < self->taskCount; index++)
    {
        if (Fero_Task_isDue(self->tasks[index], time))
        {
            Fero_Task_invoke(self->tasks[index]);
            return true;
        }
    }
    return true;
}