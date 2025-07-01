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
typedef bool (*Fero_Tasklet_Function)(void*);

typedef enum 
{
    FERO_TASKLET_INVOCATION_NONE = 0,
    FERO_TASKLET_INVOCATION_ALWAYS = 1,
    FERO_TASKLET_INVOCATION_PERIODIC = 2,
    FERO_TASKLET_INVOCATION_QUEUE = 3,
} Fero_Tasklet_InvocationType;

/*---DATA STRUCTURES---*/

/**
 * @brief Queue for storing messages passed between tasklets.
*/
typedef struct {
    uint32_t itemSize;
    uint32_t capacity;
    uint32_t nextIndex;
    uint32_t lastIndex;
    uint8_t *backingBuffer;
    uint32_t count;
    uint32_t* sizes;
} Fero_Queue;

/**
 * @brief Tasklet definition, containing pointer to user code and scheduling information.
*/
typedef struct {
    Fero_Name name;
    Fero_Tasklet_InvocationType invocationType;
    Fero_Tasklet_Function tasklet;
    void* taskletData;
    Fero_Queue* queue;
    Fero_TimeNs period;
    Fero_TimeNs offset;
    Fero_TimeNs nextActivationTime;
    uint32_t priority;
} Fero_Tasklet;

/**
 * @brief Scheduler, responsible for scheduling tasklet invocations
 */

typedef struct {
    uint32_t taskletCapacity;
    uint32_t taskletCount;
    uint8_t* buffer;
    Fero_Tasklet** tasklets;
} Fero_Scheduler;

/*---QUEUE---*/

/**
 * @brief Initialize a queue structure
 *
 * This function initializes a queue with the specified capacity, maximum item size and buffer.
 *
 * @param self Pointer to the queue to initialize
 * @param capacity Maximum number of items the queue can hold
 * @param itemSize Maximum size in bytes of each item in the queue
 * @param buffer Pre-allocated, alligned buffer for queue storage (use FERO_QUEUE_BUFFER)
 * @return true on successful initialization, false on failure
 */
bool Fero_Queue_init(
    Fero_Queue* self,
    const uint32_t capacity,
    const uint32_t itemSize,
    uint8_t* buffer
);

/**
 * @brief Gets the current number of elements in the queue.
 *
 * @param self Pointer to the queue instance.
 * @return uint32_t The number of elements currently in the queue.
 */
uint32_t Fero_Queue_getCount(
    Fero_Queue* self
);

/**
 * @brief Adds an item to a queue.
 *
 * This function attempts to put data from the provided buffer into the queue.
 *
 * @param self Pointer to the queue instance.
 * @param buffer Pointer to the data buffer to be added to the queue.
 * @param size Size of the data in bytes to be added to the queue.
 *
 * @return true if the data was successfully added to the queue,
 *         false if the operation failed (e.g., queue is full).
 */
bool Fero_Queue_put(
    Fero_Queue* self,
    uint8_t* buffer,
    const uint32_t size
);

/**
 * @brief Retrieves an item from the queue.
 *
 * This function dequeues an item from the specified queue and copies it into the provided buffer.
 *
 * @param self Pointer to the queue instance.
 * @param buffer Pointer to the buffer where the dequeued item will be stored.
 * @param size Pointer to a variable that will be updated with the actual size of the dequeued item.
 *
 * @return true if an item was successfully retrieved from the queue.
 * @return false if the queue is empty.
 */
bool Fero_Queue_get(
    Fero_Queue* self,
    uint8_t* buffer,
    uint32_t* size
);

/**
 * @brief Peeks an item from the queue.
 *
 * This function peeks an item from the specified queue and copies it into the provided buffer.
 * The item is not removed from the queue.
 *
 * @param self Pointer to the queue instance.
 * @param buffer Pointer to the buffer where the peeked item will be stored.
 * @param size Pointer to a variable that will be updated with the actual size of the peeked item.
 *
 * @return true if an item was successfully peeked from the queue.
 * @return false if the queue is empty.
 */
bool Fero_Queue_peek(
    Fero_Queue* self,
    uint8_t* buffer,
    uint32_t* size
);

/*---TASKLET---*/

/**
 * @brief Initializes a tasklet with a name and function.
 *
 * This function initializes the provided tasklet structure with the specified name
 * and tasklet function.
 *
 * @param self Pointer to the tasklet structure to initialize.
 * @param name Name to assign to the tasklet.
 * @param tasklet Function pointer to the tasklet implementation.
 * @param taskletData User data to pass to the tasklet function.
 * 
 * @return true if initialization was successful, false otherwise.
 */
bool Fero_Tasklet_init(
    Fero_Tasklet* self,
    char* name,
    Fero_Tasklet_Function tasklet,
    void* taskletData
);

/**
 * @brief Configures a tasklet to always be in the active state.
 * 
 * When a tasklet is configured to always active, it will be considered for execution in every
 * scheduler cycle.
 * 
 * @param self Pointer to the tasklet to configure
 * @return true if the tasklet was successfully set to always active, false otherwise
 */
bool Fero_Tasklet_setAlwaysActive(
    Fero_Tasklet* self
);

/**
 * @brief Configures a tasklet to run periodically with a specified period and offset.
 *
 * Configures a tasklet to execute at regular intervals defined by the period,
 * with an initial time offset from system start.
 *
 * @param self Pointer to the tasklet to configure
 * @param period The time interval between consecutive executions (in nanoseconds)
 * @param offset The time offset from system start for the first execution (in nanoseconds)
 * @return true if the tasklet was successfully set to periodic mode, false otherwise
 */
bool Fero_Tasklet_setPeriodic(
    Fero_Tasklet* self,
    const Fero_TimeNs period,
    const Fero_TimeNs offset
);

/**
 * @brief Configures a tasklet to be executed only if the provided queue is not empty.
 *
 * @param self Pointer to the tasklet instance
 * @param queue Pointer to the queue to activate this tasklet
 * @return true if the tasklet was succesfully set to be activated by the queue, false otherwise
 */
bool Fero_Tasklet_setQueueActivated(
    Fero_Tasklet* self,
    Fero_Queue* queue
);

/**
 * @brief Checks if a tasklet is due for execution at a given time.
 *
 * This function determines whether the specified tasklet is scheduled for execution.
 * Tasklet may be due if:
 * -its time is due for periodic tasklets
 * -the associated queue is not empty for queue activated tasklets
 * -it is always active
 *
 * @param self Pointer to the tasklet to check.
 * @param time The current time in nanoseconds to compare against the tasklet's scheduled time.
 * @return true if the tasklet is due for execution, false otherwise.
 */
bool Fero_Tasklet_isDue(
    Fero_Tasklet* self,
    const Fero_TimeNs time
);

/**
 * @brief Invokes a tasklet.
 *
 * Executes the tasklet pointed to by the given pointer.
 * This also updates the next activation time for periodic tasklets.
 *
 * @param self Pointer to the tasklet to be invoked.
 * @return true if the tasklet execution was successful, false otherwise.
 */
bool Fero_Tasklet_invoke(
    Fero_Tasklet* self
);

/*---SCHEDULER---*/

/**
 * @brief Initializes a scheduler instance
 *
 * @param self Pointer to the scheduler instance to be initialized
 * @param taskletCapacity Maximum number of tasklets the scheduler can manage
 * @param buffer Memory buffer to be used for scheduler's internal data structures
 * 
 * @return true if initialization was successful
 * @return false if initialization failed
 */
bool Fero_Scheduler_init(
    Fero_Scheduler* self,
    const uint32_t taskletCapacity,
    uint8_t* buffer
);


/**
 * @brief Adds a tasklet to the scheduler with specified priority.
 *
 * This function adds the provided tasklet to the scheduler's tasklet queue with the given priority.
 * Higher priority tasklets will be executed before lower priority ones.
 *
 * @param self Pointer to the scheduler instance.
 * @param tasklet Pointer to the tasklet to be added to the scheduler.
 * @param priority Priority level of the tasklet, higher values indicate higher priority.
 *
 * @return true if the tasklet was successfully added to the scheduler, false otherwise.
 */
bool Fero_Scheduler_addTasklet(
    Fero_Scheduler* self, 
    Fero_Tasklet* tasklet,
    const uint32_t priority
);


/**
 * @brief Invokes the scheduler at a specified time
 *
 * This function runs the scheduler at the given time, processing any tasklets that are due to be
 * executed at or before that time.
 *
 * @param self Pointer to the scheduler instance
 * @param time The current time in nanoseconds
 * @return true
 */
bool Fero_Scheduler_invoke(
    Fero_Scheduler* self,
    const Fero_TimeNs time
);


#endif