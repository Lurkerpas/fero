#include "../src/fero.h"
#include "../libs/Unity/src/unity.h"

/* Dummy function and counter to test task invocations */
static uint32_t task_counter = 0;
static uint32_t task_counter2 = 0;

static bool increase_task_counter();
static bool increase_task_counter2();

static bool increase_task_counter()
{
    task_counter++;
    return true;
}

static bool increase_task_counter2()
{
    task_counter2++;
    return true;
}

void setUp()
{
    task_counter = 0;
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

void Fero_Task_init_works()
{
    Fero_Task task;
    Fero_Name name = "TestTask";
    
    TEST_ASSERT_TRUE(Fero_Task_init(&task, name, increase_task_counter));
    
    TEST_ASSERT_EQUAL_STRING_LEN(name, task.name, FERO_NAME_SIZE);
    TEST_ASSERT_EQUAL_PTR(increase_task_counter, task.tasklet);
    TEST_ASSERT_EQUAL_INT(FERO_TASK_INVOCATION_NONE, task.invocationType);
    TEST_ASSERT_NULL(task.queue);
    TEST_ASSERT_EQUAL_UINT64(0, task.period);
    TEST_ASSERT_EQUAL_UINT64(0, task.offset);
    TEST_ASSERT_EQUAL_UINT64(0, task.deadline);
    TEST_ASSERT_EQUAL_UINT64(0, task.nextActivationTime);
    TEST_ASSERT_EQUAL_UINT32(0, task.priority);
}

void Fero_Task_setAlwaysActive_works()
{
    Fero_Task task;
    Fero_Name name = "TestTask";
    Fero_Task_init(&task, name, NULL);
    
    TEST_ASSERT_TRUE(Fero_Task_setAlwaysActive(&task));
    TEST_ASSERT_EQUAL_INT(FERO_TASK_INVOCATION_ALWAYS, task.invocationType);
}

void Fero_Task_setPeriodic_works()
{
    Fero_Task task;
    Fero_Name name = "TestTask";
    Fero_Task_init(&task, name, NULL);
    
    Fero_TimeNs period = 1000000000;  // 1 second
    Fero_TimeNs offset = 500000000;   // 0.5 seconds
    
    TEST_ASSERT_TRUE(Fero_Task_setPeriodic(&task, period, offset));
    TEST_ASSERT_EQUAL_INT(FERO_TASK_INVOCATION_PERIODIC, task.invocationType);
    TEST_ASSERT_EQUAL_UINT64(period, task.period);
    TEST_ASSERT_EQUAL_UINT64(offset, task.offset);
    TEST_ASSERT_EQUAL_UINT64(offset, task.nextActivationTime);
}

void Fero_Task_setQueueActivated_works()
{
    Fero_Task task;
    Fero_Name name = "TestTask";
    Fero_Task_init(&task, name, NULL);
    
    FERO_QUEUE_BUFFER(buffer, 2, 10);
    Fero_Queue queue;
    Fero_Queue_init(&queue, 2, 10, buffer);
    
    TEST_ASSERT_TRUE(Fero_Task_setQueueActivated(&task, &queue));
    TEST_ASSERT_EQUAL_INT(FERO_TASK_INVOCATION_QUEUE, task.invocationType);
    TEST_ASSERT_EQUAL_PTR(&queue, task.queue);
}

void Fero_Task_isDue_works_for_always()
{
    Fero_Task task;
    Fero_Name name = "AlwaysTask";
    Fero_Task_init(&task, name, NULL);
    Fero_Task_setAlwaysActive(&task);
    
    TEST_ASSERT_TRUE(Fero_Task_isDue(&task, 0));
    TEST_ASSERT_TRUE(Fero_Task_isDue(&task, 1000000000));
    TEST_ASSERT_TRUE(Fero_Task_isDue(&task, 2000000000));
}

void Fero_Task_isDue_works_for_periodic()
{
    Fero_Task task;
    Fero_Name name = "PeriodicTask";
    Fero_Task_init(&task, name, NULL);
    
    Fero_TimeNs period = 10;
    Fero_TimeNs offset = 5;
    Fero_Task_setPeriodic(&task, period, offset);
    
    // Not yet due (before offset)
    TEST_ASSERT_FALSE(Fero_Task_isDue(&task, 0));
    TEST_ASSERT_FALSE(Fero_Task_isDue(&task, 4));
    
    // Due at offset
    TEST_ASSERT_TRUE(Fero_Task_isDue(&task, 5));
    TEST_ASSERT_TRUE(Fero_Task_isDue(&task, 15));
}

void Fero_Task_isDue_works_for_queue()
{
    Fero_Task task;
    Fero_Name name = "QueueTask";
    Fero_Task_init(&task, name, NULL);
    
    FERO_QUEUE_BUFFER(buffer, 2, 10);
    Fero_Queue queue;
    Fero_Queue_init(&queue, 2, 10, buffer);
    Fero_Task_setQueueActivated(&task, &queue);
    
    // Empty queue - not due
    TEST_ASSERT_FALSE(Fero_Task_isDue(&task, 0));
    
    // Add item to queue - should be due
    uint8_t data[] = {1, 2, 3, 4};
    Fero_Queue_put(&queue, data, sizeof(data));
    TEST_ASSERT_TRUE(Fero_Task_isDue(&task, 0));
}

void Fero_Task_invoke_works()
{
    Fero_Task task;
    Fero_Name name = "TestTask";
    Fero_Task_init(&task, name, increase_task_counter);
    
    TEST_ASSERT_EQUAL_UINT32(0, task_counter);
    TEST_ASSERT_TRUE(Fero_Task_invoke(&task));
    TEST_ASSERT_EQUAL_UINT32(1, task_counter);
    TEST_ASSERT_TRUE(Fero_Task_invoke(&task));
    TEST_ASSERT_EQUAL_UINT32(2, task_counter);
}

void Fero_Task_invoke_updates_periodic_task_time()
{
    Fero_Task task;
    Fero_Name name = "PeriodicTask";
    Fero_Task_init(&task, name, increase_task_counter);
    
    Fero_TimeNs period = 1000000000;  // 1 second
    Fero_TimeNs offset = 500000000;   // 0.5 seconds
    Fero_Task_setPeriodic(&task, period, offset);
    
    TEST_ASSERT_TRUE(Fero_Task_invoke(&task));
    TEST_ASSERT_EQUAL_UINT64(offset + period, task.nextActivationTime);
    
    TEST_ASSERT_TRUE(Fero_Task_invoke(&task));
    TEST_ASSERT_EQUAL_UINT64(offset + period + period, task.nextActivationTime);
}

void Fero_Scheduler_init_works()
{
    FERO_SCHEDULER_BUFFER(buffer, 10);
    Fero_Scheduler scheduler;
    
    TEST_ASSERT_TRUE(Fero_Scheduler_init(&scheduler, 10, buffer));

    TEST_ASSERT_EQUAL_UINT32(10, scheduler.taskCapacity);
    TEST_ASSERT_EQUAL_UINT32(0, scheduler.taskCount);
    TEST_ASSERT_EQUAL_PTR(buffer, scheduler.buffer);
    TEST_ASSERT_EQUAL_PTR(buffer, (uint8_t*)scheduler.tasks);
}

void Fero_Scheduler_init_fails_on_misalignment()
{
    FERO_SCHEDULER_BUFFER(buffer, 10);
    Fero_Scheduler scheduler;
    uint8_t* misaligned_ptr = buffer + 1; // Force misalignment

    TEST_ASSERT_FALSE(Fero_Scheduler_init(&scheduler, 10, misaligned_ptr));
}

void Fero_Scheduler_addTask_works()
{
    FERO_SCHEDULER_BUFFER(buffer, 5);
    Fero_Scheduler scheduler;
    Fero_Scheduler_init(&scheduler, 5, buffer);
    
    Fero_Task task1;
    Fero_Name name1 = "Task1";
    Fero_Task_init(&task1, name1, increase_task_counter);
    
    TEST_ASSERT_TRUE(Fero_Scheduler_addTask(&scheduler, &task1, 5));
    TEST_ASSERT_EQUAL_UINT32(1, scheduler.taskCount);
    TEST_ASSERT_EQUAL_PTR(&task1, scheduler.tasks[0]);
    TEST_ASSERT_EQUAL_UINT32(5, task1.priority);
}

void Fero_Scheduler_addTask_fails_when_full()
{
    FERO_SCHEDULER_BUFFER(buffer, 1);
    Fero_Scheduler scheduler;
    Fero_Scheduler_init(&scheduler, 1, buffer);
    
    Fero_Task task1;
    Fero_Name name1 = "Task1";
    Fero_Task_init(&task1, name1, increase_task_counter);
    
    Fero_Task task2;
    Fero_Name name2 = "Task2";
    Fero_Task_init(&task2, name2, increase_task_counter);
    
    TEST_ASSERT_TRUE(Fero_Scheduler_addTask(&scheduler, &task1, 1));
    TEST_ASSERT_FALSE(Fero_Scheduler_addTask(&scheduler, &task2, 2));
    TEST_ASSERT_EQUAL_UINT32(1, scheduler.taskCount);
}

void Fero_Scheduler_addTask_sorts_by_priority()
{
    FERO_SCHEDULER_BUFFER(buffer, 3);
    Fero_Scheduler scheduler;
    Fero_Scheduler_init(&scheduler, 3, buffer);
    
    Fero_Task task1;
    Fero_Name name1 = "Task1";
    Fero_Task_init(&task1, name1, increase_task_counter);
    
    Fero_Task task2;
    Fero_Name name2 = "Task2";
    Fero_Task_init(&task2, name2, increase_task_counter);
    
    Fero_Task task3;
    Fero_Name name3 = "Task3";
    Fero_Task_init(&task3, name3, increase_task_counter);
    
    // Add in incorrect priority order
    TEST_ASSERT_TRUE(Fero_Scheduler_addTask(&scheduler, &task1, 1));
    TEST_ASSERT_TRUE(Fero_Scheduler_addTask(&scheduler, &task2, 3));
    TEST_ASSERT_TRUE(Fero_Scheduler_addTask(&scheduler, &task3, 2));
    
    // Verify tasks are sorted by priority (highest first)
    TEST_ASSERT_EQUAL_PTR(&task2, scheduler.tasks[0]);
    TEST_ASSERT_EQUAL_PTR(&task3, scheduler.tasks[1]);
    TEST_ASSERT_EQUAL_PTR(&task1, scheduler.tasks[2]);
}

void Fero_Scheduler_invoke_works()
{
    FERO_SCHEDULER_BUFFER(buffer, 3);
    Fero_Scheduler scheduler;
    Fero_Scheduler_init(&scheduler, 3, buffer);
    
    Fero_Task task;
    Fero_Name name = "Task";
    Fero_Task_init(&task, name, increase_task_counter);
    Fero_Task_setAlwaysActive(&task);
    
    TEST_ASSERT_TRUE(Fero_Scheduler_addTask(&scheduler, &task, 1));
    
    TEST_ASSERT_EQUAL_UINT32(0, task_counter);
    TEST_ASSERT_TRUE(Fero_Scheduler_invoke(&scheduler, 0));
    TEST_ASSERT_EQUAL_UINT32(1, task_counter);
}

void Fero_Scheduler_invoke_respects_priority()
{
    FERO_SCHEDULER_BUFFER(buffer, 2);
    Fero_Scheduler scheduler;
    Fero_Scheduler_init(&scheduler, 2, buffer);
    
    // Setup two always active tasks
    Fero_Task lowPriorityTask;
    Fero_Name lowName = "LowTask";
    Fero_Task_init(&lowPriorityTask, lowName, increase_task_counter);
    Fero_Task_setAlwaysActive(&lowPriorityTask);
    
    Fero_Task highPriorityTask;
    Fero_Name highName = "HighTask";
    Fero_Task_init(&highPriorityTask, highName, increase_task_counter2);
    Fero_Task_setAlwaysActive(&highPriorityTask);
    
    // Add low priority first, then high priority
    TEST_ASSERT_TRUE(Fero_Scheduler_addTask(&scheduler, &lowPriorityTask, 1));
    TEST_ASSERT_TRUE(Fero_Scheduler_addTask(&scheduler, &highPriorityTask, 2));
    
    // Invoke should call the high priority task
    TEST_ASSERT_EQUAL_UINT32(0, task_counter);
    TEST_ASSERT_EQUAL_UINT32(0, task_counter2);
    
    TEST_ASSERT_TRUE(Fero_Scheduler_invoke(&scheduler, 0));
    
    TEST_ASSERT_EQUAL_UINT32(0, task_counter);
    TEST_ASSERT_EQUAL_UINT32(1, task_counter2);
}

void Fero_Scheduler_invoke_handles_periodic_tasks()
{
    FERO_SCHEDULER_BUFFER(buffer, 1);
    Fero_Scheduler scheduler;
    Fero_Scheduler_init(&scheduler, 1, buffer);
    
    Fero_Task periodicTask;
    Fero_Name name = "PeriodicTask";
    Fero_Task_init(&periodicTask, name, increase_task_counter);
    
    Fero_TimeNs period = 10;
    Fero_TimeNs offset = 5;
    Fero_Task_setPeriodic(&periodicTask, period, offset);
    
    TEST_ASSERT_TRUE(Fero_Scheduler_addTask(&scheduler, &periodicTask, 1));
    
    // Before offset - task should not run
    TEST_ASSERT_EQUAL_UINT32(0, task_counter);
    TEST_ASSERT_TRUE(Fero_Scheduler_invoke(&scheduler, 0));
    TEST_ASSERT_EQUAL_UINT32(0, task_counter);
    
    // At offset - task should run
    TEST_ASSERT_TRUE(Fero_Scheduler_invoke(&scheduler, 5));
    TEST_ASSERT_EQUAL_UINT32(1, task_counter);
    
    // After first execution but before next period - should not run
    TEST_ASSERT_TRUE(Fero_Scheduler_invoke(&scheduler, 10));
    TEST_ASSERT_EQUAL_UINT32(1, task_counter);
    
    // At next period - should run
    TEST_ASSERT_TRUE(Fero_Scheduler_invoke(&scheduler, 15));
    TEST_ASSERT_EQUAL_UINT32(2, task_counter);
}

void Fero_Scheduler_invoke_handles_queue_tasks()
{
    FERO_SCHEDULER_BUFFER(buffer, 1);
    Fero_Scheduler scheduler;
    Fero_Scheduler_init(&scheduler, 1, buffer);
    
    FERO_QUEUE_BUFFER(queueBuffer, 2, 10);
    Fero_Queue queue;
    Fero_Queue_init(&queue, 2, 10, queueBuffer);
    
    Fero_Task queueTask;
    Fero_Name name = "QueueTask";
    Fero_Task_init(&queueTask, name, increase_task_counter);
    Fero_Task_setQueueActivated(&queueTask, &queue);
    
    TEST_ASSERT_TRUE(Fero_Scheduler_addTask(&scheduler, &queueTask, 1));
    
    // Empty queue - task should not run
    TEST_ASSERT_EQUAL_UINT32(0, task_counter);
    TEST_ASSERT_TRUE(Fero_Scheduler_invoke(&scheduler, 0));
    TEST_ASSERT_EQUAL_UINT32(0, task_counter);
    
    // Add item to queue - task should run
    uint8_t data[] = {1, 2, 3};
    Fero_Queue_put(&queue, data, sizeof(data));
    TEST_ASSERT_TRUE(Fero_Scheduler_invoke(&scheduler, 0));
    TEST_ASSERT_EQUAL_UINT32(1, task_counter);
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
    RUN_TEST(Fero_Task_init_works);
    RUN_TEST(Fero_Task_setAlwaysActive_works);
    RUN_TEST(Fero_Task_setPeriodic_works);
    RUN_TEST(Fero_Task_setQueueActivated_works);
    RUN_TEST(Fero_Task_isDue_works_for_always);
    RUN_TEST(Fero_Task_isDue_works_for_periodic);
    RUN_TEST(Fero_Task_isDue_works_for_queue);
    RUN_TEST(Fero_Task_invoke_works);
    RUN_TEST(Fero_Task_invoke_updates_periodic_task_time);
    RUN_TEST(Fero_Scheduler_init_works);
    RUN_TEST(Fero_Scheduler_init_fails_on_misalignment);
    RUN_TEST(Fero_Scheduler_addTask_works);
    RUN_TEST(Fero_Scheduler_addTask_fails_when_full);
    RUN_TEST(Fero_Scheduler_addTask_sorts_by_priority);
    RUN_TEST(Fero_Scheduler_invoke_works);
    RUN_TEST(Fero_Scheduler_invoke_respects_priority);
    RUN_TEST(Fero_Scheduler_invoke_handles_periodic_tasks);
    RUN_TEST(Fero_Scheduler_invoke_handles_queue_tasks);
    return UNITY_END();
}