#include "tinylsl/sample.h"
#include "macrotools/error.h"
#include <stdbool.h>
#include <string.h>

int lsl_sample_value_serialize(const lsl_sample_value_t sample_value, const lsl_channel_info_t *channel_info, uint8_t *buf) {
    // put the generic sample value into a uint64 (which will fit all sizes)
    uint64_t value;
    switch (channel_info->format) {
        case LSL_FLOAT32: {
            memcpy(&value, &sample_value.float32_value, 4);
            break;
        }
        case LSL_DOUBLE64:
            memcpy(&value, &sample_value.double64_value, 8);
            break;
        case LSL_INT32:
            value = (uint64_t) sample_value.int32_value;
        case LSL_INT16:
            value = (uint64_t) sample_value.int16_value;
        case LSL_INT8:
            value = (uint64_t) sample_value.int8_value;
        case LSL_INT64:
            value = (uint64_t) sample_value.int64_value;
            break;
        default:
            LOG_ERROR("Format %d is not an integral type.", channel_info->format);
            return 1;
    }

    // serialize the required number of bits from the uint64
    const size_t n = lsl_integral_sizes[channel_info->format];
    for (uint64_t i = 0; i < n; i++) {
        const uint64_t j = channel_info->order == LSL_LSB_FIRST ? i : (n - i - 1);
        buf[i] = (value >> (j * 8)) & 0xff;
    }

    return 0;
}

int lsl_sample_serialize(const lsl_sample_t *sample, const lsl_channel_info_t *channel_info, uint8_t *buf, size_t len) {
    const bool timestamp_deduced = sample->timestamp == LSL_DEDUCED_TIMESTAMP;

    // validate the buffer size
    const size_t required_len = 1 + (timestamp_deduced ? 0 : 8) + channel_info->num_channels * lsl_integral_sizes[channel_info->format];
    TRY_OR_RETURN(TRY_ASSERT(len >= required_len), "Buffer too small. Expected at least %zu bytes, but was %zu.", required_len, len);

    // serialize the timestamp
    *buf++ = timestamp_deduced ? 1 : 2;
    if (!timestamp_deduced) {
        const lsl_sample_value_t timestamp_value = { .double64_value = sample->timestamp };
        TRY_OR_RETURN(lsl_sample_value_serialize(timestamp_value, &((lsl_channel_info_t) {
            .format = LSL_DOUBLE64,
            .order = LSL_MSB_FIRST,
            .num_channels = 1,
        }), buf), "Failed to serialize timestamp.");
        buf += lsl_integral_sizes[LSL_DOUBLE64];
    }

    // serialize each sample
    for (int i = 0; i < channel_info->num_channels; i++) {
        TRY_OR_RETURN(lsl_sample_value_serialize(sample->values[i], channel_info, buf), "Failed to serialize sample value.");
        buf += lsl_integral_sizes[channel_info->format];
    }

    return 0;
}
