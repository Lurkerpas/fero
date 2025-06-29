#include "fero.h"
#include <string.h>

bool Fero_Queue_init(
    Fero_Queue* self,
    const uint32_t capacity,
    const uint32_t itemSize,
    uint8_t* buffer
)
{
    if ((uintptr_t)buffer % 4 != 0)
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

uint32_t Fero_Queue_count(
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