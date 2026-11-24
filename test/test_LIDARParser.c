#ifdef TEST

#include <string.h>
#include "unity.h"
#include "LIDARParser.h"

#define DIMENSION_OF(a) (sizeof(a)/sizeof(a[0]))

extern Parser_State_t Parserstate;
static Msg_Data_t msgTest;

void setUp(void)
{
    memset(&msgTest, 0, sizeof msgTest);
    Parserstate = PARSER_LOOKING_FOR_START;
}

void tearDown(void)
{
    Parser_Reset();
}

/* Feeds a whole frame byte by byte, returns the status of the last byte */
static Return_Status_t sendMessage(uint8_t *msg, uint8_t len)
{
    int i;
    for (i = 0; i < len - 1; i++)
    {
        Parser_AddByte(&msgTest, msg[i]);
    }
    return Parser_AddByte(&msgTest, msg[i]);
}



void test_Parser_ResetsIfInvalidHeadByte(void)
{
    Parser_AddByte(&msgTest, RX_DATA_HEAD_BYTE);
    Parser_AddByte(&msgTest, 'Z');
    TEST_ASSERT_EQUAL(PARSER_LOOKING_FOR_START, Parserstate);

    Parser_AddByte(&msgTest, 'Z');
    Parser_AddByte(&msgTest, RX_DATA_HEAD_BYTE);
    TEST_ASSERT_EQUAL(PARSER_LOOKING_FOR_START, Parserstate);
}

void test_Parser_LooksForDataAfterValidHeadBytes(void)
{
    Parser_AddByte(&msgTest, RX_DATA_HEAD_BYTE);
    Parser_AddByte(&msgTest, RX_DATA_HEAD_BYTE);
    TEST_ASSERT_EQUAL(PARSER_LOOKING_FOR_DATA, Parserstate);
}

void test_Parser_LooksForChecksumAfterData(void)
{
    uint8_t msg[] = {0x59, 0x59, 0x04, 0x04, 0x02, 0x02, 0x05, 0x05};
    sendMessage(msg, DIMENSION_OF(msg));
    TEST_ASSERT_EQUAL(PARSER_LOOKING_FOR_CHECKSUM, Parserstate);
}

void test_Parser_DecodesFieldsAfterValidFrame(void)
{
    uint8_t msg[] = {0x59, 0x59, 0x04, 0x04, 0x02, 0x02, 0x05, 0x05, 0xC8};
    sendMessage(msg, DIMENSION_OF(msg));
    TEST_ASSERT_EQUAL_HEX16(0x0404, msgTest.dist);
    TEST_ASSERT_EQUAL_HEX16(0x0202, msgTest.amps);
    TEST_ASSERT_EQUAL_HEX16(0x0505, msgTest.temp);
}

void test_Parser_GoodFrameDecodesAfterBadFrame(void)
{
    uint8_t bad[]  = {0x59, 0x59, 0x04, 0x04, 0x02, 0x02, 0x05, 0x05, 0x00};
    uint8_t good[] = {0x59, 0x59, 0x64, 0x00, 0x90, 0x01, 0xC8, 0x08, 0x77};
    sendMessage(bad, DIMENSION_OF(bad));
    sendMessage(good, DIMENSION_OF(good));
    TEST_ASSERT_EQUAL_HEX16(100, msgTest.dist);
}

void test_Parser_TwoFramesBackToBackBothDecode(void)
{
    uint8_t a[] = {0x59, 0x59, 0x64, 0x00, 0x90, 0x01, 0xC8, 0x08, 0x77};
    uint8_t b[] = {0x59, 0x59, 0x59, 0x00, 0x90, 0x01, 0xC8, 0x08, 0x6C};
    sendMessage(a, DIMENSION_OF(a));
    TEST_ASSERT_EQUAL_HEX16(100, msgTest.dist);
    sendMessage(b, DIMENSION_OF(b));
    TEST_ASSERT_EQUAL_HEX16(89, msgTest.dist);
}

void test_Parser_BadChecksumLeavesMessageUnchanged(void)
{
    msgTest.dist = 0xAAAA;
    msgTest.amps = 0xBBBB;
    msgTest.temp = 0xCCCC;
    uint8_t msg[] = {0x59, 0x59, 0x64, 0x00, 0x90, 0x01, 0xC8, 0x08, 0x78}; /* 0x77 is correct */
    sendMessage(msg, DIMENSION_OF(msg));
    TEST_ASSERT_EQUAL_HEX16(0xAAAA, msgTest.dist);
    TEST_ASSERT_EQUAL_HEX16(0xBBBB, msgTest.amps);
    TEST_ASSERT_EQUAL_HEX16(0xCCCC, msgTest.temp);
}

void test_Parser_ZeroChecksumIsRejectedWhenSumIsNotZero(void)
{
    uint8_t msg[] = {0x59, 0x59, 0x04, 0x04, 0x02, 0x02, 0x05, 0x05, 0x00};
    msgTest.dist = 0xAAAA;
    sendMessage(msg, DIMENSION_OF(msg));
    TEST_ASSERT_EQUAL_HEX16(0xAAAA, msgTest.dist);
}

void test_Parser_ChecksumAboveFFIsTruncatedTo8Bits(void)
{
    /* sum of the first 8 bytes is 0x277, so the expected checksum byte is 0x77 */
    uint8_t msg[] = {0x59, 0x59, 0x64, 0x00, 0x90, 0x01, 0xC8, 0x08, 0x77};
    sendMessage(msg, DIMENSION_OF(msg));
    TEST_ASSERT_EQUAL_HEX16(100, msgTest.dist);
    TEST_ASSERT_EQUAL_HEX16(400, msgTest.amps);
    TEST_ASSERT_EQUAL_HEX16(0x08C8, msgTest.temp);
}

void test_Parser_HeaderByteInsidePayloadDoesNotDisturbFrame(void)
{
    uint8_t msg[] = {0x59, 0x59, 0x59, 0x00, 0x90, 0x01, 0xC8, 0x08, 0x6C};
    sendMessage(msg, DIMENSION_OF(msg));
    TEST_ASSERT_EQUAL_HEX16(89, msgTest.dist);
}

void test_Parser_DecodesHighBytesLittleEndian(void)
{
    uint8_t msg[] = {0x59, 0x59, 0x23, 0x01, 0x90, 0x01, 0xC8, 0x08, 0x37};
    sendMessage(msg, DIMENSION_OF(msg));
    TEST_ASSERT_EQUAL_HEX16(0x0123, msgTest.dist);
}

void test_Parser_ReturnsErrorIfChecksumIncorrect(void)
{
    uint8_t msg[] = {0x59, 0x59, 0x64, 0x00, 0x90, 0x01, 0xC8, 0x08, 0x78}; /* 0x77 is correct */
    TEST_ASSERT_EQUAL(PARSER_CHECKSUM_ERROR, sendMessage(msg, DIMENSION_OF(msg)));
}

#endif // TEST