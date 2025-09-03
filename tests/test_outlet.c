#include "macrotools/test.h"
#include "tinylsl/outlet.h"


DESCRIBE(test_outlet, "outlet") {
    lsl_outlet_t outlet;
    uint8_t sample_buffer[1024];

    IT("initializes an outlet") {
        EXPECT_EQ(lsl_outlet_create(&outlet, &((lsl_outlet_config_t) {
            .channel_info = {
                .format = LSL_INT32,
                .order = LSL_LSB_FIRST,
                .num_channels = 8,
            },
        }), sample_buffer, sizeof(sample_buffer)), 0);
    }

    IT("checks the number of channels") {
        EXPECT_NE(lsl_outlet_create(&outlet, &((lsl_outlet_config_t) {
            .channel_info = {
                .format = LSL_INT32,
                .order = LSL_LSB_FIRST,
                .num_channels = 0,
            },
        }), sample_buffer, sizeof(sample_buffer)), 0);
    }

    IT("checks the required buffer size") {
        EXPECT_NE(lsl_outlet_create(&outlet, &((lsl_outlet_config_t) {
            .channel_info = {
                .format = LSL_INT32,
                .order = LSL_LSB_FIRST,
                .num_channels = 8,
            },
        }), NULL, sizeof(sample_buffer)), 0);

        EXPECT_EQ(lsl_outlet_create(&outlet, &((lsl_outlet_config_t) {
            .channel_info = {
                .format = LSL_INT32,
                .order = LSL_LSB_FIRST,
                .num_channels = 8,
            },
        }), sample_buffer, sizeof(lsl_sample_t) + 8 * sizeof(lsl_sample_value_t)), 0);

        EXPECT_NE(lsl_outlet_create(&outlet, &((lsl_outlet_config_t) {
            .channel_info = {
                .format = LSL_INT32,
                .order = LSL_LSB_FIRST,
                .num_channels = 8,
            },
        }), sample_buffer, sizeof(lsl_sample_t) + 8 * sizeof(lsl_sample_value_t) - 1), 0);
    }
}
