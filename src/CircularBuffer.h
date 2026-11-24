#ifndef CIRCULARBUFFER_H
#define CIRCULARBUFFER_H

#include <stdint.h>
#include <stdbool.h>

#define BUFF_SIZE (26+1) //Extra byte used by CircularBuffer_IsFull()

typedef struct Circular_Buffer *circBuff;

//Select type of data to be stored by circular buffer
typedef uint16_t Item;

circBuff CircularBuffer_Init(void);
bool CircularBuffer_Put(circBuff c, Item data);
bool CircularBuffer_Get(circBuff c, Item *data);
bool CircularBuffer_IsEmpty(circBuff c);
bool CircularBuffer_IsFull(circBuff c);
bool CircularBuffer_IsHalfFull(circBuff c);
void CircularBuffer_Clear(circBuff c);
bool CircularBuffer_FindMean(circBuff c, Item *avg);

#ifdef TEST
uint32_t getHead(circBuff c);
uint32_t getTail(circBuff c);
Item *getBuff(circBuff c);
#endif

#endif // CIRCULARBUFFER_H
