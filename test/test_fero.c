#include "../src/fero.h"
#include "../libs/Unity/src/unity.h"

/* Dummy function and counter to test tasklet invocations */
static uint32_t tasklet_counter = 0;
static uint32_t tasklet_counter2 = 0;

static bool increase_tasklet_counter();
static bool increase_tasklet_counter2();

static bool increase_tasklet_counter()
{
    tasklet_counter++;
    return true;
}

static bool increase_tasklet_counter2()
{
    tasklet_counter2++;
    return true;
}

void setUp()
{
    tasklet_counter = 0;
}

void tearDown()
{

}

void Fero_Queue_init_works()
{
    FERO_QUEUE_BUFFER(buffer, 10, 20);
    Fero_Queue queue;
    
    TEST_ASSERT_TRUE(Fero_Queue_init(&queue, 10, 20, buffer));

    TEST_ASSERT_EQUAL_UINT32(10, queue.capacity);
    TEST_ASSERT_EQUAL_UINT32(20, queue.itemSize);
    TEST_ASSERT_EQUAL_PTR(buffer, queue.backingBuffer);
    TEST_ASSERT_EQUAL_UINT32(0, queue.nextIndex);
    TEST_ASSERT_EQUAL_UINT32(0, queue.lastIndex);
    TEST_ASSERT_EQUAL_UINT32(0, queue.count);
}

void Fero_Queue_init_fails_on_misalignment()
{
    FERO_QUEUE_BUFFER(buffer, 10, 20);
    Fero_Queue queue;
    uint8_t* misaligned_ptr = buffer + 1; // Force misalignment

    TEST_ASSERT_FALSE(Fero_Queue_init(&queue, 10, 20, misaligned_ptr));
}

void Fero_Queue_getCount_works()
{
    FERO_QUEUE_BUFFER(buffer, 10, 20);
    Fero_Queue queue;
    Fero_Queue_init(&queue, 10, 20, buffer);
    
    TEST_ASSERT_EQUAL_UINT32(0, Fero_Queue_getCount(&queue));
    
    uint8_t data[] = {1, 2, 3, 4};
    Fero_Queue_put(&queue, data, sizeof(data));
    TEST_ASSERT_EQUAL_UINT32(1, Fero_Queue_getCount(&queue));
    
    Fero_Queue_put(&queue, data, sizeof(data));
    TEST_ASSERT_EQUAL_UINT32(2, Fero_Queue_getCount(&queue));
    
    uint8_t outBuffer[20];
    uint32_t outSize;
    Fero_Queue_get(&queue, outBuffer, &outSize);
    TEST_ASSERT_EQUAL_UINT32(1, Fero_Queue_getCount(&queue));
}

void Fero_Queue_put_works()
{
    FERO_QUEUE_BUFFER(buffer, 2, 20);
    Fero_Queue queue;
    Fero_Queue_init(&queue, 2, 20, buffer);
    uint8_t data1[] = {1, 2, 3, 4};

    TEST_ASSERT_TRUE(Fero_Queue_put(&queue, data1, sizeof(data1)));

    TEST_ASSERT_EQUAL_UINT32(1, Fero_Queue_getCount(&queue));
}

void Fero_Queue_put_fails_when_item_too_big()
{
    FERO_QUEUE_BUFFER(buffer, 2, 10);
    Fero_Queue queue;
    Fero_Queue_init(&queue, 2, 10, buffer);
    uint8_t largeData[15] = {0};

    TEST_ASSERT_FALSE(Fero_Queue_put(&queue, largeData, sizeof(largeData)));

    TEST_ASSERT_EQUAL_UINT32(0, Fero_Queue_getCount(&queue));
}

void Fero_Queue_put_fails_when_full()
{
    FERO_QUEUE_BUFFER(buffer, 2, 20);
    Fero_Queue queue;
    Fero_Queue_init(&queue, 2, 20, buffer);

    uint8_t data2[] = {5, 6, 7};
    TEST_ASSERT_TRUE(Fero_Queue_put(&queue, data2, sizeof(data2)));
    uint8_t data3[] = {8, 9, 10};
    TEST_ASSERT_TRUE(Fero_Queue_put(&queue, data3, sizeof(data3)));
    TEST_ASSERT_EQUAL_UINT32(2, Fero_Queue_getCount(&queue));
    
    uint8_t data4[] = {11, 12};
    TEST_ASSERT_FALSE(Fero_Queue_put(&queue, data4, sizeof(data4)));
    TEST_ASSERT_EQUAL_UINT32(2, Fero_Queue_getCount(&queue));
}

void Fero_Queue_get_works()
{
    FERO_QUEUE_BUFFER(buffer, 3, 10);
    Fero_Queue queue;
    Fero_Queue_init(&queue, 3, 10, buffer);
    
    uint8_t outBuffer[10];
    uint32_t outSize;
    TEST_ASSERT_FALSE(Fero_Queue_get(&queue, outBuffer, &outSize));
    
    uint8_t data1[] = {1, 2, 3, 4};
    Fero_Queue_put(&queue, data1, sizeof(data1));
    uint8_t data2[] = {5, 6, 7};
    Fero_Queue_put(&queue, data2, sizeof(data2));
    
    TEST_ASSERT_TRUE(Fero_Queue_get(&queue, outBuffer, &outSize));
    TEST_ASSERT_EQUAL_UINT32(sizeof(data1), outSize);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(data1, outBuffer, sizeof(data1));
    TEST_ASSERT_EQUAL_UINT32(1, Fero_Queue_getCount(&queue));
    
    TEST_ASSERT_TRUE(Fero_Queue_get(&queue, outBuffer, &outSize));
    TEST_ASSERT_EQUAL_UINT32(sizeof(data2), outSize);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(data2, outBuffer, sizeof(data2));
    TEST_ASSERT_EQUAL_UINT32(0, Fero_Queue_getCount(&queue));
    
    TEST_ASSERT_FALSE(Fero_Queue_get(&queue, outBuffer, &outSize));
}

void Fero_Queue_get_works_with_wraparound()
{
    FERO_QUEUE_BUFFER(buffer, 2, 10);
    Fero_Queue queue;
    Fero_Queue_init(&queue, 2, 10, buffer);
    
    uint8_t outBuffer[10];
    uint32_t outSize;
    TEST_ASSERT_FALSE(Fero_Queue_get(&queue, outBuffer, &outSize));
    
    uint8_t data1[] = {1};
    TEST_ASSERT_TRUE(Fero_Queue_put(&queue, data1, sizeof(data1)));
    uint8_t data2[] = {2, 3};
    TEST_ASSERT_TRUE(Fero_Queue_put(&queue, data2, sizeof(data2)));
    // Remove one to empty space
    TEST_ASSERT_TRUE(Fero_Queue_get(&queue, outBuffer, &outSize));
    uint8_t data3[] = {4, 5, 6};
    // Put one that would fail without the wraparound
    TEST_ASSERT_TRUE(Fero_Queue_put(&queue, data3, sizeof(data3)));
    TEST_ASSERT_TRUE(Fero_Queue_get(&queue, outBuffer, &outSize));
    TEST_ASSERT_TRUE(Fero_Queue_get(&queue, outBuffer, &outSize));

    TEST_ASSERT_EQUAL_UINT32(sizeof(data3), outSize);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(data3, outBuffer, sizeof(data3));
    TEST_ASSERT_EQUAL_UINT32(0, Fero_Queue_getCount(&queue));
    TEST_ASSERT_FALSE(Fero_Queue_get(&queue, outBuffer, &outSize));
}

void Fero_Tasklet_init_works()
{
    Fero_Tasklet tasklet;
    Fero_Name name = "TestTasklet";
    
    TEST_ASSERT_TRUE(Fero_Tasklet_init(&tasklet, name, increase_tasklet_counter));
    
    TEST_ASSERT_EQUAL_STRING_LEN(name, tasklet.name, FERO_NAME_SIZE);
    TEST_ASSERT_EQUAL_PTR(increase_tasklet_counter, tasklet.tasklet);
    TEST_ASSERT_EQUAL_INT(FERO_TASKLET_INVOCATION_NONE, tasklet.invocationType);
    TEST_ASSERT_NULL(tasklet.queue);
    TEST_ASSERT_EQUAL_UINT64(0, tasklet.period);
    TEST_ASSERT_EQUAL_UINT64(0, tasklet.offset);
    TEST_ASSERT_EQUAL_UINT64(0, tasklet.deadline);
    TEST_ASSERT_EQUAL_UINT64(0, tasklet.nextActivationTime);
    TEST_ASSERT_EQUAL_UINT32(0, tasklet.priority);
}

void Fero_Tasklet_setAlwaysActive_works()
{
    Fero_Tasklet tasklet;
    Fero_Name name = "TestTasklet";
    Fero_Tasklet_init(&tasklet, name, NULL);
    
    TEST_ASSERT_TRUE(Fero_Tasklet_setAlwaysActive(&tasklet));
    TEST_ASSERT_EQUAL_INT(FERO_TASKLET_INVOCATION_ALWAYS, tasklet.invocationType);
}

void Fero_Tasklet_setPeriodic_works()
{
    Fero_Tasklet tasklet;
    Fero_Name name = "TestTasklet";
    Fero_Tasklet_init(&tasklet, name, NULL);
    
    Fero_TimeNs period = 1000000000;  // 1 second
    Fero_TimeNs offset = 500000000;   // 0.5 seconds
    
    TEST_ASSERT_TRUE(Fero_Tasklet_setPeriodic(&tasklet, period, offset));
    TEST_ASSERT_EQUAL_INT(FERO_TASKLET_INVOCATION_PERIODIC, tasklet.invocationType);
    TEST_ASSERT_EQUAL_UINT64(period, tasklet.period);
    TEST_ASSERT_EQUAL_UINT64(offset, tasklet.offset);
    TEST_ASSERT_EQUAL_UINT64(offset, tasklet.nextActivationTime);
}

void Fero_Tasklet_setQueueActivated_works()
{
    Fero_Tasklet tasklet;
    Fero_Name name = "TestTasklet";
    Fero_Tasklet_init(&tasklet, name, NULL);
    
    FERO_QUEUE_BUFFER(buffer, 2, 10);
    Fero_Queue queue;
    Fero_Queue_init(&queue, 2, 10, buffer);
    
    TEST_ASSERT_TRUE(Fero_Tasklet_setQueueActivated(&tasklet, &queue));
    TEST_ASSERT_EQUAL_INT(FERO_TASKLET_INVOCATION_QUEUE, tasklet.invocationType);
    TEST_ASSERT_EQUAL_PTR(&queue, tasklet.queue);
}

void Fero_Tasklet_isDue_works_for_always()
{
    Fero_Tasklet tasklet;
    Fero_Name name = "AlwaysTasklet";
    Fero_Tasklet_init(&tasklet, name, NULL);
    Fero_Tasklet_setAlwaysActive(&tasklet);
    
    TEST_ASSERT_TRUE(Fero_Tasklet_isDue(&tasklet, 0));
    TEST_ASSERT_TRUE(Fero_Tasklet_isDue(&tasklet, 1000000000));
    TEST_ASSERT_TRUE(Fero_Tasklet_isDue(&tasklet, 2000000000));
}

void Fero_Tasklet_isDue_works_for_periodic()
{
    Fero_Tasklet tasklet;
    Fero_Name name = "PeriodicTasklet";
    Fero_Tasklet_init(&tasklet, name, NULL);
    
    Fero_TimeNs period = 10;
    Fero_TimeNs offset = 5;
    Fero_Tasklet_setPeriodic(&tasklet, period, offset);
    
    // Not yet due (before offset)
    TEST_ASSERT_FALSE(Fero_Tasklet_isDue(&tasklet, 0));
    TEST_ASSERT_FALSE(Fero_Tasklet_isDue(&tasklet, 4));
    
    // Due at offset
    TEST_ASSERT_TRUE(Fero_Tasklet_isDue(&tasklet, 5));
    TEST_ASSERT_TRUE(Fero_Tasklet_isDue(&tasklet, 15));
}

void Fero_Tasklet_isDue_works_for_queue()
{
    Fero_Tasklet tasklet;
    Fero_Name name = "QueueTasklet";
    Fero_Tasklet_init(&tasklet, name, NULL);
    
    FERO_QUEUE_BUFFER(buffer, 2, 10);
    Fero_Queue queue;
    Fero_Queue_init(&queue, 2, 10, buffer);
    Fero_Tasklet_setQueueActivated(&tasklet, &queue);
    
    // Empty queue - not due
    TEST_ASSERT_FALSE(Fero_Tasklet_isDue(&tasklet, 0));
    
    // Add item to queue - should be due
    uint8_t data[] = {1, 2, 3, 4};
    Fero_Queue_put(&queue, data, sizeof(data));
    TEST_ASSERT_TRUE(Fero_Tasklet_isDue(&tasklet, 0));
}

void Fero_Tasklet_invoke_works()
{
    Fero_Tasklet tasklet;
    Fero_Name name = "TestTasklet";
    Fero_Tasklet_init(&tasklet, name, increase_tasklet_counter);
    
    TEST_ASSERT_EQUAL_UINT32(0, tasklet_counter);
    TEST_ASSERT_TRUE(Fero_Tasklet_invoke(&tasklet));
    TEST_ASSERT_EQUAL_UINT32(1, tasklet_counter);
    TEST_ASSERT_TRUE(Fero_Tasklet_invoke(&tasklet));
    TEST_ASSERT_EQUAL_UINT32(2, tasklet_counter);
}

void Fero_Tasklet_invoke_updates_periodic_tasklet_time()
{
    Fero_Tasklet tasklet;
    Fero_Name name = "PeriodicTasklet";
    Fero_Tasklet_init(&tasklet, name, increase_tasklet_counter);
    
    Fero_TimeNs period = 1000000000;  // 1 second
    Fero_TimeNs offset = 500000000;   // 0.5 seconds
    Fero_Tasklet_setPeriodic(&tasklet, period, offset);
    
    TEST_ASSERT_TRUE(Fero_Tasklet_invoke(&tasklet));
    TEST_ASSERT_EQUAL_UINT64(offset + period, tasklet.nextActivationTime);
    
    TEST_ASSERT_TRUE(Fero_Tasklet_invoke(&tasklet));
    TEST_ASSERT_EQUAL_UINT64(offset + period + period, tasklet.nextActivationTime);
}

void Fero_Scheduler_init_works()
{
    FERO_SCHEDULER_BUFFER(buffer, 10);
    Fero_Scheduler scheduler;
    
    TEST_ASSERT_TRUE(Fero_Scheduler_init(&scheduler, 10, buffer));

    TEST_ASSERT_EQUAL_UINT32(10, scheduler.taskletCapacity);
    TEST_ASSERT_EQUAL_UINT32(0, scheduler.taskletCount);
    TEST_ASSERT_EQUAL_PTR(buffer, scheduler.buffer);
    TEST_ASSERT_EQUAL_PTR(buffer, (uint8_t*)scheduler.tasklets);
}

void Fero_Scheduler_init_fails_on_misalignment()
{
    FERO_SCHEDULER_BUFFER(buffer, 10);
    Fero_Scheduler scheduler;
    uint8_t* misaligned_ptr = buffer + 1; // Force misalignment

    TEST_ASSERT_FALSE(Fero_Scheduler_init(&scheduler, 10, misaligned_ptr));
}

void Fero_Scheduler_addTasklet_works()
{
    FERO_SCHEDULER_BUFFER(buffer, 5);
    Fero_Scheduler scheduler;
    Fero_Scheduler_init(&scheduler, 5, buffer);
    
    Fero_Tasklet tasklet1;
    Fero_Name name1 = "Tasklet1";
    Fero_Tasklet_init(&tasklet1, name1, increase_tasklet_counter);
    
    TEST_ASSERT_TRUE(Fero_Scheduler_addTasklet(&scheduler, &tasklet1, 5));
    TEST_ASSERT_EQUAL_UINT32(1, scheduler.taskletCount);
    TEST_ASSERT_EQUAL_PTR(&tasklet1, scheduler.tasklets[0]);
    TEST_ASSERT_EQUAL_UINT32(5, tasklet1.priority);
}

void Fero_Scheduler_addTasklet_fails_when_full()
{
    FERO_SCHEDULER_BUFFER(buffer, 1);
    Fero_Scheduler scheduler;
    Fero_Scheduler_init(&scheduler, 1, buffer);
    
    Fero_Tasklet tasklet1;
    Fero_Name name1 = "Tasklet1";
    Fero_Tasklet_init(&tasklet1, name1, increase_tasklet_counter);
    
    Fero_Tasklet tasklet2;
    Fero_Name name2 = "Tasklet2";
    Fero_Tasklet_init(&tasklet2, name2, increase_tasklet_counter);
    
    TEST_ASSERT_TRUE(Fero_Scheduler_addTasklet(&scheduler, &tasklet1, 1));
    TEST_ASSERT_FALSE(Fero_Scheduler_addTasklet(&scheduler, &tasklet2, 2));
    TEST_ASSERT_EQUAL_UINT32(1, scheduler.taskletCount);
}

void Fero_Scheduler_addTasklet_sorts_by_priority()
{
    FERO_SCHEDULER_BUFFER(buffer, 3);
    Fero_Scheduler scheduler;
    Fero_Scheduler_init(&scheduler, 3, buffer);
    
    Fero_Tasklet tasklet1;
    Fero_Name name1 = "Tasklet1";
    Fero_Tasklet_init(&tasklet1, name1, increase_tasklet_counter);
    
    Fero_Tasklet tasklet2;
    Fero_Name name2 = "Tasklet2";
    Fero_Tasklet_init(&tasklet2, name2, increase_tasklet_counter);
    
    Fero_Tasklet tasklet3;
    Fero_Name name3 = "Tasklet3";
    Fero_Tasklet_init(&tasklet3, name3, increase_tasklet_counter);
    
    // Add in incorrect priority order
    TEST_ASSERT_TRUE(Fero_Scheduler_addTasklet(&scheduler, &tasklet1, 1));
    TEST_ASSERT_TRUE(Fero_Scheduler_addTasklet(&scheduler, &tasklet2, 3));
    TEST_ASSERT_TRUE(Fero_Scheduler_addTasklet(&scheduler, &tasklet3, 2));
    
    // Verify tasklets are sorted by priority (highest first)
    TEST_ASSERT_EQUAL_PTR(&tasklet2, scheduler.tasklets[0]);
    TEST_ASSERT_EQUAL_PTR(&tasklet3, scheduler.tasklets[1]);
    TEST_ASSERT_EQUAL_PTR(&tasklet1, scheduler.tasklets[2]);
}

void Fero_Scheduler_invoke_works()
{
    FERO_SCHEDULER_BUFFER(buffer, 3);
    Fero_Scheduler scheduler;
    Fero_Scheduler_init(&scheduler, 3, buffer);
    
    Fero_Tasklet tasklet;
    Fero_Name name = "Tasklet";
    Fero_Tasklet_init(&tasklet, name, increase_tasklet_counter);
    Fero_Tasklet_setAlwaysActive(&tasklet);
    
    TEST_ASSERT_TRUE(Fero_Scheduler_addTasklet(&scheduler, &tasklet, 1));
    
    TEST_ASSERT_EQUAL_UINT32(0, tasklet_counter);
    TEST_ASSERT_TRUE(Fero_Scheduler_invoke(&scheduler, 0));
    TEST_ASSERT_EQUAL_UINT32(1, tasklet_counter);
}

void Fero_Scheduler_invoke_respects_priority()
{
    FERO_SCHEDULER_BUFFER(buffer, 2);
    Fero_Scheduler scheduler;
    Fero_Scheduler_init(&scheduler, 2, buffer);
    
    // Setup two always active tasklets
    Fero_Tasklet lowPriorityTasklet;
    Fero_Name lowName = "LowTasklet";
    Fero_Tasklet_init(&lowPriorityTasklet, lowName, increase_tasklet_counter);
    Fero_Tasklet_setAlwaysActive(&lowPriorityTasklet);
    
    Fero_Tasklet highPriorityTasklet;
    Fero_Name highName = "HighTasklet";
    Fero_Tasklet_init(&highPriorityTasklet, highName, increase_tasklet_counter2);
    Fero_Tasklet_setAlwaysActive(&highPriorityTasklet);
    
    // Add low priority first, then high priority
    TEST_ASSERT_TRUE(Fero_Scheduler_addTasklet(&scheduler, &lowPriorityTasklet, 1));
    TEST_ASSERT_TRUE(Fero_Scheduler_addTasklet(&scheduler, &highPriorityTasklet, 2));
    
    // Invoke should call the high priority tasklet
    TEST_ASSERT_EQUAL_UINT32(0, tasklet_counter);
    TEST_ASSERT_EQUAL_UINT32(0, tasklet_counter2);
    
    TEST_ASSERT_TRUE(Fero_Scheduler_invoke(&scheduler, 0));
    
    TEST_ASSERT_EQUAL_UINT32(0, tasklet_counter);
    TEST_ASSERT_EQUAL_UINT32(1, tasklet_counter2);
}

void Fero_Scheduler_invoke_handles_periodic_tasklets()
{
    FERO_SCHEDULER_BUFFER(buffer, 1);
    Fero_Scheduler scheduler;
    Fero_Scheduler_init(&scheduler, 1, buffer);
    
    Fero_Tasklet periodicTasklet;
    Fero_Name name = "PeriodicTasklet";
    Fero_Tasklet_init(&periodicTasklet, name, increase_tasklet_counter);
    
    Fero_TimeNs period = 10;
    Fero_TimeNs offset = 5;
    Fero_Tasklet_setPeriodic(&periodicTasklet, period, offset);
    
    TEST_ASSERT_TRUE(Fero_Scheduler_addTasklet(&scheduler, &periodicTasklet, 1));
    
    // Before offset - tasklet should not run
    TEST_ASSERT_EQUAL_UINT32(0, tasklet_counter);
    TEST_ASSERT_TRUE(Fero_Scheduler_invoke(&scheduler, 0));
    TEST_ASSERT_EQUAL_UINT32(0, tasklet_counter);
    
    // At offset - tasklet should run
    TEST_ASSERT_TRUE(Fero_Scheduler_invoke(&scheduler, 5));
    TEST_ASSERT_EQUAL_UINT32(1, tasklet_counter);
    
    // After first execution but before next period - should not run
    TEST_ASSERT_TRUE(Fero_Scheduler_invoke(&scheduler, 10));
    TEST_ASSERT_EQUAL_UINT32(1, tasklet_counter);
    
    // At next period - should run
    TEST_ASSERT_TRUE(Fero_Scheduler_invoke(&scheduler, 15));
    TEST_ASSERT_EQUAL_UINT32(2, tasklet_counter);
}

void Fero_Scheduler_invoke_handles_queue_tasklets()
{
    FERO_SCHEDULER_BUFFER(buffer, 1);
    Fero_Scheduler scheduler;
    Fero_Scheduler_init(&scheduler, 1, buffer);
    
    FERO_QUEUE_BUFFER(queueBuffer, 2, 10);
    Fero_Queue queue;
    Fero_Queue_init(&queue, 2, 10, queueBuffer);
    
    Fero_Tasklet queueTasklet;
    Fero_Name name = "QueueTasklet";
    Fero_Tasklet_init(&queueTasklet, name, increase_tasklet_counter);
    Fero_Tasklet_setQueueActivated(&queueTasklet, &queue);
    
    TEST_ASSERT_TRUE(Fero_Scheduler_addTasklet(&scheduler, &queueTasklet, 1));
    
    // Empty queue - tasklet should not run
    TEST_ASSERT_EQUAL_UINT32(0, tasklet_counter);
    TEST_ASSERT_TRUE(Fero_Scheduler_invoke(&scheduler, 0));
    TEST_ASSERT_EQUAL_UINT32(0, tasklet_counter);
    
    // Add item to queue - tasklet should run
    uint8_t data[] = {1, 2, 3};
    Fero_Queue_put(&queue, data, sizeof(data));
    TEST_ASSERT_TRUE(Fero_Scheduler_invoke(&scheduler, 0));
    TEST_ASSERT_EQUAL_UINT32(1, tasklet_counter);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(Fero_Queue_init_works);
    RUN_TEST(Fero_Queue_init_fails_on_misalignment);
    RUN_TEST(Fero_Queue_getCount_works);
    RUN_TEST(Fero_Queue_put_works);
    RUN_TEST(Fero_Queue_put_fails_when_item_too_big);
    RUN_TEST(Fero_Queue_put_fails_when_full);
    RUN_TEST(Fero_Queue_get_works);
    RUN_TEST(Fero_Queue_get_works_with_wraparound);
    RUN_TEST(Fero_Tasklet_init_works);
    RUN_TEST(Fero_Tasklet_setAlwaysActive_works);
    RUN_TEST(Fero_Tasklet_setPeriodic_works);
    RUN_TEST(Fero_Tasklet_setQueueActivated_works);
    RUN_TEST(Fero_Tasklet_isDue_works_for_always);
    RUN_TEST(Fero_Tasklet_isDue_works_for_periodic);
    RUN_TEST(Fero_Tasklet_isDue_works_for_queue);
    RUN_TEST(Fero_Tasklet_invoke_works);
    RUN_TEST(Fero_Tasklet_invoke_updates_periodic_tasklet_time);
    RUN_TEST(Fero_Scheduler_init_works);
    RUN_TEST(Fero_Scheduler_init_fails_on_misalignment);
    RUN_TEST(Fero_Scheduler_addTasklet_works);
    RUN_TEST(Fero_Scheduler_addTasklet_fails_when_full);
    RUN_TEST(Fero_Scheduler_addTasklet_sorts_by_priority);
    RUN_TEST(Fero_Scheduler_invoke_works);
    RUN_TEST(Fero_Scheduler_invoke_respects_priority);
    RUN_TEST(Fero_Scheduler_invoke_handles_periodic_tasklets);
    RUN_TEST(Fero_Scheduler_invoke_handles_queue_tasklets);
    return UNITY_END();
}