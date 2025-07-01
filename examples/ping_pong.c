#include <fero.h>
#include <stdio.h>
#include <time.h>

FERO_QUEUE_BUFFER(pingQueueBuffer, 1, sizeof(uint32_t));
FERO_QUEUE_BUFFER(pongQueueBuffer, 1, sizeof(uint32_t));
FERO_SCHEDULER_BUFFER(schedulerBuffer, 2);
Fero_Queue pingQueue;
Fero_Queue pongQueue;
Fero_Tasklet pingTasklet;
Fero_Tasklet pongTasklet;
Fero_Scheduler scheduler;

static inline Fero_TimeNs getCurrentTime() 
{
    uint64_t ns;
    struct timespec spec;

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &spec);
    ns = spec.tv_sec * 1000000000 + spec.tv_nsec;
}

static bool ping(void* data)
{
    uint32_t counter;
    if (Fero_Queue_getCount(&pingQueue) == 0)
    {
        return true;
    }
    uint32_t size;
    Fero_Queue_get(&pingQueue, (uint8_t*)&counter, &size);
    printf("Received ping %d\n", counter);
    counter++;
    Fero_Queue_put(&pongQueue, (uint8_t*)&counter, size);
    return true;
}

static bool pong(void* data)
{
    uint32_t counter;
    if (Fero_Queue_getCount(&pongQueue) == 0)
    {
        return true;
    }
    uint32_t size;
    Fero_Queue_get(&pongQueue, (uint8_t*)&counter, &size);
    printf("Received pong %d\n", counter);
    counter++;
    Fero_Queue_put(&pingQueue, (uint8_t*)&counter, size);
    return true;
}

int main(void)
{
    Fero_Queue_init(&pingQueue, 1, sizeof(uint32_t), pingQueueBuffer);
    Fero_Queue_init(&pongQueue, 1, sizeof(uint32_t), pongQueueBuffer);

    Fero_Tasklet_init(&pingTasklet, "Ping", ping, NULL);
    Fero_Tasklet_setQueueActivated(&pingTasklet, &pingQueue);
    Fero_Tasklet_init(&pongTasklet, "Pong", pong, NULL);
    Fero_Tasklet_setQueueActivated(&pongTasklet, &pongQueue);

    Fero_Scheduler_init(&scheduler, 2, schedulerBuffer);
    Fero_Scheduler_addTasklet(&scheduler, &pingTasklet, 1);
    Fero_Scheduler_addTasklet(&scheduler, &pongTasklet, 1);

    // Send the first ping to kick-off the exchange
    uint32_t counter = 0;
    uint32_t size = 0;
    Fero_Queue_put(&pingQueue, (uint8_t*)&counter, sizeof(uint32_t));

    while(true)
    {
        Fero_Scheduler_invoke(&scheduler, getCurrentTime());
        // Peek at the queue to see whether we reached a limit
        if (Fero_Queue_peek(&pingQueue, (uint8_t*)&counter, &size))
        {
            if (counter > 32)
            {
                // End program execution
                return 0;
            }
        }
    }
    return 0;
}