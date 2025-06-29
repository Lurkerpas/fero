#include "../src/fero.h"
#include "../libs/Unity/src/unity.h"

void setUp()
{

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

void Fero_Queue_count_works()
{
    FERO_QUEUE_BUFFER(buffer, 10, 20);
    Fero_Queue queue;
    Fero_Queue_init(&queue, 10, 20, buffer);
    
    TEST_ASSERT_EQUAL_UINT32(0, Fero_Queue_count(&queue));
    
    uint8_t data[] = {1, 2, 3, 4};
    Fero_Queue_put(&queue, data, sizeof(data));
    TEST_ASSERT_EQUAL_UINT32(1, Fero_Queue_count(&queue));
    
    Fero_Queue_put(&queue, data, sizeof(data));
    TEST_ASSERT_EQUAL_UINT32(2, Fero_Queue_count(&queue));
    
    uint8_t outBuffer[20];
    uint32_t outSize;
    Fero_Queue_get(&queue, outBuffer, &outSize);
    TEST_ASSERT_EQUAL_UINT32(1, Fero_Queue_count(&queue));
}

void Fero_Queue_put_works()
{
    FERO_QUEUE_BUFFER(buffer, 2, 20);
    Fero_Queue queue;
    Fero_Queue_init(&queue, 2, 20, buffer);
    uint8_t data1[] = {1, 2, 3, 4};

    TEST_ASSERT_TRUE(Fero_Queue_put(&queue, data1, sizeof(data1)));

    TEST_ASSERT_EQUAL_UINT32(1, Fero_Queue_count(&queue));
}

void Fero_Queue_put_fails_when_item_too_big()
{
    FERO_QUEUE_BUFFER(buffer, 2, 10);
    Fero_Queue queue;
    Fero_Queue_init(&queue, 2, 10, buffer);
    uint8_t largeData[15] = {0};

    TEST_ASSERT_FALSE(Fero_Queue_put(&queue, largeData, sizeof(largeData)));

    TEST_ASSERT_EQUAL_UINT32(0, Fero_Queue_count(&queue));
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
    TEST_ASSERT_EQUAL_UINT32(2, Fero_Queue_count(&queue));
    
    uint8_t data4[] = {11, 12};
    TEST_ASSERT_FALSE(Fero_Queue_put(&queue, data4, sizeof(data4)));
    TEST_ASSERT_EQUAL_UINT32(2, Fero_Queue_count(&queue));
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
    TEST_ASSERT_EQUAL_UINT32(1, Fero_Queue_count(&queue));
    
    TEST_ASSERT_TRUE(Fero_Queue_get(&queue, outBuffer, &outSize));
    TEST_ASSERT_EQUAL_UINT32(sizeof(data2), outSize);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(data2, outBuffer, sizeof(data2));
    TEST_ASSERT_EQUAL_UINT32(0, Fero_Queue_count(&queue));
    
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
    TEST_ASSERT_EQUAL_UINT32(0, Fero_Queue_count(&queue));
    TEST_ASSERT_FALSE(Fero_Queue_get(&queue, outBuffer, &outSize));
}


int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(Fero_Queue_init_works);
    RUN_TEST(Fero_Queue_init_fails_on_misalignment);
    RUN_TEST(Fero_Queue_count_works);
    RUN_TEST(Fero_Queue_put_works);
    RUN_TEST(Fero_Queue_put_fails_when_item_too_big);
    RUN_TEST(Fero_Queue_put_fails_when_full);
    RUN_TEST(Fero_Queue_get_works);
    RUN_TEST(Fero_Queue_get_works_with_wraparound);
    return UNITY_END();
}