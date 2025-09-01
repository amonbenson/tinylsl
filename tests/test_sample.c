#include "test_macros.h"
#include "tinylsl/sample.h"


DESCRIBE(test_sample, "sample") {
    uint8_t buffer[32];

    uint8_t sample_buffer[128];
    lsl_sample_t *test_sample = (lsl_sample_t *) sample_buffer;

    IT("serializes individual values") {
        EXPECT_EQ(lsl_sample_value_serialize(
            ((lsl_sample_value_t) { .int8_value = 123 }),
            &((lsl_channel_info_t) {
                .format = LSL_INT8,
                .order = LSL_LSB_FIRST,
                .num_channels = 8,
            }),
            buffer
        ), 0);
        EXPECT_EQ(buffer[0], 123);

        EXPECT_EQ(lsl_sample_value_serialize(
            ((lsl_sample_value_t) { .int32_value = 0x01234567 }),
            &((lsl_channel_info_t) {
                .format = LSL_INT32,
                .order = LSL_LSB_FIRST,
                .num_channels = 8,
            }),
            buffer
        ), 0);
        EXPECT_EQ(buffer[0], 0x67);
        EXPECT_EQ(buffer[1], 0x45);
        EXPECT_EQ(buffer[2], 0x23);
        EXPECT_EQ(buffer[3], 0x01);

        EXPECT_EQ(lsl_sample_value_serialize(
            ((lsl_sample_value_t) { .int32_value = 0x01234567 }),
            &((lsl_channel_info_t) {
                .format = LSL_INT32,
                .order = LSL_MSB_FIRST,
                .num_channels = 8,
            }),
            buffer
        ), 0);
        EXPECT_EQ(buffer[0], 0x01);
        EXPECT_EQ(buffer[1], 0x23);
        EXPECT_EQ(buffer[2], 0x45);
        EXPECT_EQ(buffer[3], 0x67);

        EXPECT_EQ(lsl_sample_value_serialize(
            ((lsl_sample_value_t) { .float32_value = 2.25 }),
            &((lsl_channel_info_t) {
                .format = LSL_FLOAT32,
                .order = LSL_MSB_FIRST,
                .num_channels = 8,
            }),
            buffer
        ), 0);
        EXPECT_EQ(buffer[0], 0x40);
        EXPECT_EQ(buffer[1], 0x10);
        EXPECT_EQ(buffer[2], 0x00);
        EXPECT_EQ(buffer[3], 0x00);
    }

    IT("serializes timestamp") {
        test_sample->timestamp = LSL_DEDUCED_TIMESTAMP;
        test_sample->values[0] = (lsl_sample_value_t) { .int32_value = 123 };

        EXPECT_EQ(lsl_sample_serialize(
            test_sample,
            &((lsl_channel_info_t) {
                .format = LSL_INT32,
                .order = LSL_MSB_FIRST,
                .num_channels = 1,
            }),
            buffer,
            sizeof(buffer)
        ), 0);
        EXPECT_EQ(buffer[0], 0x01);

        test_sample->timestamp = 1756752792.123;
        EXPECT_EQ(lsl_sample_serialize(
            test_sample,
            &((lsl_channel_info_t) {
                .format = LSL_INT32,
                .order = LSL_MSB_FIRST,
                .num_channels = 1,
            }),
            buffer,
            sizeof(buffer)
        ), 0);
        EXPECT_EQ(buffer[0], 0x02);
        EXPECT_EQ(buffer[1], 0x41);
        EXPECT_EQ(buffer[8], 0x3B); // check only first and last digit
    }

    IT("serializes a whole sample") {
        test_sample->timestamp = LSL_DEDUCED_TIMESTAMP;
        test_sample->values[0] = (lsl_sample_value_t) { .int32_value = 0x01234567 };
        test_sample->values[1] = (lsl_sample_value_t) { .int32_value = 0x89abcdef };
        test_sample->values[2] = (lsl_sample_value_t) { .int32_value = 0xdeadbeef };

        EXPECT_EQ(lsl_sample_serialize(
            test_sample,
            &((lsl_channel_info_t) {
                .format = LSL_INT32,
                .order = LSL_MSB_FIRST,
                .num_channels = 3,
            }),
            buffer,
            sizeof(buffer)
        ), 0);

        EXPECT_EQ(buffer[0], 0x01);
        EXPECT_EQ(buffer[1], 0x01);
        EXPECT_EQ(buffer[4], 0x67);
        EXPECT_EQ(buffer[5], 0x89);
        EXPECT_EQ(buffer[8], 0xef);
        EXPECT_EQ(buffer[9], 0xde);
        EXPECT_EQ(buffer[12], 0xef);
    }
}
