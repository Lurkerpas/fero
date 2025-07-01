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

bool Fero_Queue_peek(
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
    return true;
}

/*---TASKLET---*/

bool Fero_Tasklet_init(
    Fero_Tasklet* self,
    char* name,
    Fero_Tasklet_Function tasklet,
    void* taskletData
)
{
    strcpy(self->name, name);
    self->tasklet = tasklet;
    self->invocationType = FERO_TASKLET_INVOCATION_NONE;
    self->queue = NULL;
    self->period = 0;
    self->offset = 0;
    self->nextActivationTime = 0;
    self->priority = 0;
    self->taskletData = taskletData;
    return true;
}

bool Fero_Tasklet_setAlwaysActive(
    Fero_Tasklet* self
)
{
    self->invocationType = FERO_TASKLET_INVOCATION_ALWAYS;
    return true;
}

bool Fero_Tasklet_setPeriodic(
    Fero_Tasklet* self,
    const Fero_TimeNs period,
    const Fero_TimeNs offset
)
{
    self->invocationType = FERO_TASKLET_INVOCATION_PERIODIC;
    self->period = period;
    self->offset = offset;
    self->nextActivationTime = offset;
    return true;
}

bool Fero_Tasklet_setQueueActivated(
    Fero_Tasklet* self,
    Fero_Queue* queue
)
{
    self->invocationType = FERO_TASKLET_INVOCATION_QUEUE;
    self->queue = queue;
    return true;
}

bool Fero_Tasklet_isDue(
    Fero_Tasklet* self,
    const Fero_TimeNs time
)
{
    switch (self->invocationType)
    {
        case FERO_TASKLET_INVOCATION_ALWAYS:
            return true;
        case FERO_TASKLET_INVOCATION_PERIODIC:
            return self->nextActivationTime <= time;
        case FERO_TASKLET_INVOCATION_QUEUE:
            return Fero_Queue_getCount(self->queue) > 0;
        default:
            return false;
    }
}

bool Fero_Tasklet_invoke(
    Fero_Tasklet* self
)
{
    if (self->invocationType == FERO_TASKLET_INVOCATION_PERIODIC)
    {
        self->nextActivationTime += self->period;
    }
    self->tasklet(self->taskletData);
    return true;
}

/*---SCHEDULER---*/

bool Fero_Scheduler_init(
    Fero_Scheduler* self,
    const uint32_t taskletCapacity,
    uint8_t* buffer
)
{
    if ((uintptr_t)buffer % FERO_ALIGNMENT_SIZE != 0)
    {
        // Alignment on 4 bytes is required
        return false;
    }
    self->taskletCapacity = taskletCapacity;
    self->taskletCount = 0;
    self->buffer = buffer;
    self->tasklets = (Fero_Tasklet**)buffer;
    return true;
}

bool Fero_Scheduler_addTasklet(
    Fero_Scheduler* self, 
    Fero_Tasklet* tasklet,
    const uint32_t priority
)
{
    if (self->taskletCount == self->taskletCapacity)
    {
        return false;
    }
    tasklet->priority = priority;
    self->taskletCount++;
    // Try to insert it before an existing one
    for (uint32_t index = 0; index < self->taskletCount - 1; index++)
    {
        if (self->tasklets[index]->priority < priority)
        {
            // Move all existing tasklets by one place
            for (uint32_t backIndex = self->taskletCount - 1; backIndex > index; backIndex--)
            {
                self->tasklets[backIndex] = self->tasklets[backIndex-1];
            }
            // Insert tasklet
            self->tasklets[index] = tasklet;
            return true;
        }
    }
    // Was not inserted before an exising one, so add it at the end
    self->tasklets[self->taskletCount - 1] = tasklet;
    return true;
}

bool Fero_Scheduler_invoke(
    Fero_Scheduler* self,
    const Fero_TimeNs time
)
{
    for (uint32_t index = 0; index < self->taskletCount; index++)
    {
        if (Fero_Tasklet_isDue(self->tasklets[index], time))
        {
            Fero_Tasklet_invoke(self->tasklets[index]);
            return true;
        }
    }
    return true;
}