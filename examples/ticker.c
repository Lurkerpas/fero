#include <fero.h>
#include <stdio.h>
#include <time.h>

#define MAX_WORK_ITEMS 16u

FERO_QUEUE_BUFFER(workBuffer, MAX_WORK_ITEMS, sizeof(uint32_t));
FERO_SCHEDULER_BUFFER(schedulerBuffer, 2);

Fero_Queue workQueue;
Fero_Tasklet workTasklet;
Fero_Tasklet tickerTasklet;
Fero_Scheduler scheduler;

static inline Fero_TimeNs getCurrentTime() 
{
    uint64_t ns;
    struct timespec spec;

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &spec);
    ns = spec.tv_sec * 1000000000 + spec.tv_nsec;
}

static bool work(void* data) 
{
    if (Fero_Queue_getCount(&workQueue) == 0)
    {
        return true;
    }
    uint32_t size = 0;
    uint32_t item = 0;
    Fero_Queue_get(&workQueue, (uint8_t*)&item, &size);
    printf("Work got item %d\n", item);
}

static bool ticker(void* data)
{
    uint32_t* context = (uint32_t*)data;
    printf("Ticker woken at %ld\n", getCurrentTime());
    for (uint32_t i = 0u; i < MAX_WORK_ITEMS; i++)
    {
        uint32_t item = i + *context;
        Fero_Queue_put(&workQueue, (uint8_t*)&item, sizeof(uint32_t));
    }
    *context += MAX_WORK_ITEMS;
}

int main(void)
{
    Fero_Queue_init(&workQueue, MAX_WORK_ITEMS, sizeof(uint32_t), workBuffer);

    uint32_t tickerContext = 0;
    Fero_Tasklet_init(&tickerTasklet, "Ticker", ticker, &tickerContext);
    Fero_Tasklet_setPeriodic(&tickerTasklet, 1000000000, 500000000);
    Fero_Tasklet_init(&workTasklet, "Work", work, NULL);
    Fero_Tasklet_setQueueActivated(&workTasklet, &workQueue);

    Fero_Scheduler_init(&scheduler, 2, schedulerBuffer);
    Fero_Scheduler_addTasklet(&scheduler, &workTasklet, 1);
    Fero_Scheduler_addTasklet(&scheduler, &tickerTasklet, 1);
    // Run for 3 seconds
    while(getCurrentTime() < 3000000000)
    {
        Fero_Scheduler_invoke(&scheduler, getCurrentTime());
    }
    return 0;
}