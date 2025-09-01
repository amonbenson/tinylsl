#include "test_macros.h"
#include "tinylsl/sample.h"


DESCRIBE(test_sample, "sample") {
    uint8_t buffer[32];

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
        printf("%02x\r\n", buffer[0]);
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
        printf("%02x\r\n", buffer[0]);
        EXPECT_EQ(buffer[0], 0x01);
        EXPECT_EQ(buffer[1], 0x23);
        EXPECT_EQ(buffer[2], 0x45);
        EXPECT_EQ(buffer[3], 0x67);
    }
}
