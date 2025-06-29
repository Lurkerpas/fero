#ifndef FERO_H
#define FERO_H

#include <stdint.h>
#include <stdbool.h>

typedef uint64_t Fer_TimeNs;

#define FERO_ALIGNED __attribute__((aligned(4)))
#define FERO_ALIGN(size) (((size + 3)/4)*4)
#define FERO_QUEUE_BUFFER_SIZE(capacity, item_size) (capacity * (FERO_ALIGN(item_size) + sizeof(uint32_t)))
#define FERO_QUEUE_BUFFER(name, capacity, item_size) uint8_t name[FERO_QUEUE_BUFFER_SIZE(capacity, item_size)] FERO_ALIGNED

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

} Fero_Task;

bool Fero_Queue_init(
    Fero_Queue* self,
    const uint32_t capacity,
    const uint32_t itemSize,
    uint8_t* buffer
);

uint32_t Fero_Queue_count(
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

#endif