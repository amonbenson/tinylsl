#include "tinylsl/sample.h"
#include "error_macros.h"
#include <stdbool.h>

int lsl_sample_value_serialize(const lsl_sample_value_t sample_value, const lsl_channel_info_t *channel_info, uint8_t *buf) {
    // put the generic sample value into a uint64 (which will fit all sizes)
    uint64_t value;
    switch (channel_info->format) {
        case LSL_FLOAT32: {
            value = *((uint32_t *) &sample_value.float32_value);
            break;
        }
        case LSL_DOUBLE64:
            value = *((uint64_t *) &sample_value.double64_value);
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
    for (int i = 0; i < n; i++) {
        const size_t j = channel_info->order == LSL_LSB_FIRST ? i : (n - i - 1);
        buf[i] = (value >> (j * 8)) & 0xff;
    }

    return 0;
}

int lsl_sample_serialize(const lsl_sample_t *sample, const lsl_channel_info_t *channel_info, uint8_t *buf, size_t len) {
    const bool timestamp_deduced = sample->timestamp == LSL_TIMESTAMP_DEDUCED;

    // validate the buffer size
    const size_t required_len = 1 + (timestamp_deduced ? 0 : 8) + channel_info->num_channels * lsl_integral_sizes[channel_info->format];
    RETURN_ON_ERROR(ASSERT(len >= required_len), "Buffer too small. Expected at least %zu bytes, but was %zu.", required_len, len);

    // serialize the timestamp
    *buf++ = timestamp_deduced ? 1 : 2;
    if (!timestamp_deduced) {
        const lsl_sample_value_t timestamp_value = { .double64_value = sample->timestamp };
        RETURN_ON_ERROR(lsl_sample_value_serialize(timestamp_value, &((lsl_channel_info_t) {
            .format = LSL_DOUBLE64,
            .order = LSL_MSB_FIRST,
            .num_channels = 1,
        }), buf), "Failed to serialize timestamp.");
        *buf += lsl_integral_sizes[LSL_DOUBLE64];
    }

    // serialize each sample
    for (int i = 0; i < channel_info->num_channels; i++) {
        RETURN_ON_ERROR(lsl_sample_value_serialize(sample->values[i], channel_info, buf), "Failed to serialize sample value.");
        *buf += lsl_integral_sizes[channel_info->format];
    }

    return 0;
}
