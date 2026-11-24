
#ifdef TEST

#include "unity.h"
#include "CircularBuffer.h"

circBuff circBuffTest;

#define DIMENSION_OF(a) (sizeof(a)/sizeof(a[0]))

void setUp(void)
{
    circBuffTest = CircularBuffer_Init();
}

void tearDown(void)
{
}

static void advance_indices(int offset)
{
    Item out;
    for (int i = 0; i < offset; i++)
    {
        CircularBuffer_Put(circBuffTest, 0);
        CircularBuffer_Get(circBuffTest, &out);
    }
}

void test_CircularBuffer_AllSetToZero(void)
{
    TEST_ASSERT_EQUAL(0, getHead(circBuffTest));
    TEST_ASSERT_EQUAL(0, getTail(circBuffTest));
    Item *buffer = getBuff(circBuffTest);
    for(int i = 0; i < BUFF_SIZE; i++)
    {
        TEST_ASSERT_EQUAL(0, buffer[i]);
    }
}

void test_CircularBuffer_PutOneByte(void)
{
    TEST_ASSERT_TRUE(CircularBuffer_Put(circBuffTest, 'A'));
    Item *buffer = getBuff(circBuffTest);
    TEST_ASSERT_EQUAL('A', buffer[getHead(circBuffTest)]);
}

void test_CircularBuffer_GetOneByte(void)
{
    Item data_read;
    TEST_ASSERT_TRUE(CircularBuffer_Put(circBuffTest, 'A'));
    TEST_ASSERT_TRUE(CircularBuffer_Get(circBuffTest, &data_read));
    TEST_ASSERT_EQUAL('A', data_read);
}

void test_CircularBuffer_PutAndGetMultipleBytes(void)
{
    Item data_read[12];
    Item msg[12] = {'h', 'e', 'l', 'l', 'o', ' ', 'w', 'o', 'r', 'l', 'd', '\0'};
    for(int i = 0; i < 12; i++)
    {
        TEST_ASSERT_TRUE(CircularBuffer_Put(circBuffTest, msg[i]));
    }
    for(int i = 0; i < 12; i++)
    {
        TEST_ASSERT_TRUE(CircularBuffer_Get(circBuffTest, &data_read[i]));
    }
    TEST_ASSERT_EQUAL_UINT8_ARRAY(msg, data_read, 12);
}

void test_CircularBuffer_IsEmpty(void)
{
    TEST_ASSERT_TRUE(CircularBuffer_IsEmpty(circBuffTest));
    CircularBuffer_Put(circBuffTest, 1);
    TEST_ASSERT_FALSE(CircularBuffer_IsEmpty(circBuffTest));
}

void test_CircularBuffer_IsFull(void)
{
    TEST_ASSERT_FALSE(CircularBuffer_IsFull(circBuffTest));
    for(int i = 0; i < BUFF_SIZE-1; i++)
    {
        CircularBuffer_Put(circBuffTest, 1);
    }
    TEST_ASSERT_TRUE(CircularBuffer_IsFull(circBuffTest));
}

void test_CircularBuffer_PutReturnsFalseIfBufferFull(void)
{
    for(int i = 0; i < BUFF_SIZE-1; i++)
    {
        TEST_ASSERT_TRUE(CircularBuffer_Put(circBuffTest, 1));
    }
    TEST_ASSERT_FALSE(CircularBuffer_Put(circBuffTest, 1));
}

void test_CircularBuffer_GetReturnsFalseIfBufferEmpty(void)
{
    Item dummy;
    TEST_ASSERT_FALSE(CircularBuffer_Get(circBuffTest, &dummy));
}

void test_CircularBuffer_HeadAndTailIndexesWrapToZero(void)
{
    Item dummy;
    for(int i = 0; i < BUFF_SIZE; i++)
    {
        CircularBuffer_Put(circBuffTest, 1);
        CircularBuffer_Get(circBuffTest, &dummy);
    }
    TEST_ASSERT_EQUAL(0, getHead(circBuffTest));
    TEST_ASSERT_EQUAL(0, getTail(circBuffTest));
}

void test_CircularBuffer_WrapAroundPreservesDataOrder(void)
{
    Item out;
    
    for (int i = 0; i < BUFF_SIZE - 1; i++)
    {
        TEST_ASSERT_TRUE(CircularBuffer_Put(circBuffTest, i));
    }

    for (int i = 0; i < BUFF_SIZE / 2; i++)
    {
        TEST_ASSERT_TRUE(CircularBuffer_Get(circBuffTest, &out));
        TEST_ASSERT_EQUAL_UINT8(i, out);
    }
    for (int i = 0; i < BUFF_SIZE / 2; i++)
    {
        TEST_ASSERT_TRUE(CircularBuffer_Put(circBuffTest, 100 + i));
    }
    
    for (int i = BUFF_SIZE / 2; i < BUFF_SIZE - 1; i++)
    {
        TEST_ASSERT_TRUE(CircularBuffer_Get(circBuffTest, &out));
        TEST_ASSERT_EQUAL_UINT8(i, out);
    }
    for (int i = 0; i < BUFF_SIZE / 2; i++)
    {
        TEST_ASSERT_TRUE(CircularBuffer_Get(circBuffTest, &out));
        TEST_ASSERT_EQUAL_UINT8(100 + i, out);
    }
    TEST_ASSERT_TRUE(CircularBuffer_IsEmpty(circBuffTest));
}

void test_CircularBuffer_ReturnTrueIfBufferHalfFull(void)
{
    TEST_ASSERT_FALSE(CircularBuffer_IsHalfFull(circBuffTest));
    Item out;
    for (int i = 0; i < BUFF_SIZE / 2; i++)
    {
        TEST_ASSERT_TRUE(CircularBuffer_Put(circBuffTest, i));
        
    }
    TEST_ASSERT_TRUE(CircularBuffer_IsHalfFull(circBuffTest));
    for (int i = 0; i < BUFF_SIZE / 2; i++)
    {
        TEST_ASSERT_TRUE(CircularBuffer_Get(circBuffTest, &out));
        TEST_ASSERT_EQUAL_UINT8(i, out);
    }
    TEST_ASSERT_FALSE(CircularBuffer_IsHalfFull(circBuffTest));
    for (int i = 0; i < BUFF_SIZE-1; i++)
    {
        TEST_ASSERT_TRUE(CircularBuffer_Put(circBuffTest, i));
        
    }
    TEST_ASSERT_TRUE(CircularBuffer_IsFull(circBuffTest));
    TEST_ASSERT_TRUE(CircularBuffer_IsHalfFull(circBuffTest));

}



void test_CircularBuffer_HalfFullBoundaryAtEveryOffset(void)
{
    for (int offset = 0; offset < BUFF_SIZE; offset++)
    {

        circBuffTest = CircularBuffer_Init();
        advance_indices(offset);
        
        for (int i = 0; i < BUFF_SIZE / 2 - 1; i++)
            CircularBuffer_Put(circBuffTest, i);
        TEST_ASSERT_FALSE_MESSAGE(CircularBuffer_IsHalfFull(circBuffTest), "one below half");

        CircularBuffer_Put(circBuffTest, 0);
        TEST_ASSERT_TRUE_MESSAGE(CircularBuffer_IsHalfFull(circBuffTest), "exactly half");
    }
}

void test_CircularBuffer_ClearBuffer(void)
{
    for(int i = 0; i < BUFF_SIZE-1; i++)
    {
        CircularBuffer_Put(circBuffTest, 1);
    }
    TEST_ASSERT_FALSE(CircularBuffer_IsEmpty(circBuffTest));
    CircularBuffer_Clear(circBuffTest);
    TEST_ASSERT_TRUE(CircularBuffer_IsEmpty(circBuffTest));
}

void test_CircularBuffer_FindMeanGivesCorrectAvg(void)
{
    uint16_t a[] = {98, 101, 99, 100, 102, 100, 99, 101};   // mean 100
    uint16_t avg;
    for(size_t i = 0; i < DIMENSION_OF(a); i++)
    {
        CircularBuffer_Put(circBuffTest, a[i]);
    }
    TEST_ASSERT_TRUE(CircularBuffer_FindMean(circBuffTest, &avg));
    TEST_ASSERT_EQUAL(100, avg);
    CircularBuffer_Clear(circBuffTest);

    TEST_ASSERT_FALSE(CircularBuffer_FindMean(circBuffTest, &avg));
}


#endif // TEST
