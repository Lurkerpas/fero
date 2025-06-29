#include "../src/fero.h"
#include "../libs/Unity/src/unity.h"

/* Dummy function and counter to test task invocations */
static uint32_t task_counter = 0;

static bool increase_task_counter();

static bool increase_task_counter()
{
    task_counter++;
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
    return UNITY_END();
}