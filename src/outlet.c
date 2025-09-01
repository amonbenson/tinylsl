#include "tinylsl/outlet.h"
#include "error_macros.h"

int lsl_outlet_create(lsl_outlet_t *outlet, const lsl_outlet_config_t *config, uint8_t *sample_buffer, size_t sample_buffer_len) {
    // validate configuration
    RETURN_ON_ERROR(ASSERT(config->channel_info.num_channels > 0), "Invalid number of channels: %lu", config->channel_info.num_channels);
    RETURN_ON_ERROR(ASSERT(config->channel_info.format >= 0 && config->channel_info.format < 8), "Invalid channel format: %d", config->channel_info.format);
    RETURN_ON_ERROR(ASSERT(config->channel_info.order == LSL_LSB_FIRST || config->channel_info.order == LSL_MSB_FIRST), "Invalid byte order: %d", config->channel_info.order);

    // validate sample buffer
    const size_t required_buffer_len = sizeof(lsl_sample_t) + config->channel_info.num_channels * sizeof(lsl_sample_value_t);
    RETURN_ON_ERROR(ASSERT(sample_buffer_len >= required_buffer_len), "Sample buffer is not large enough (required at least %lu bytes, but was %lu)", required_buffer_len, sample_buffer_len);
    RETURN_ON_ERROR(ASSERT(sample_buffer != NULL), "Sample buffer must not be NULL.");

    // init struct
    outlet->config = *config;
    outlet->sample_buffer = sample_buffer;

    return 0;
}

int lsl_outlet_destroy(lsl_outlet_t *outlet) {
    return 0;
}

lsl_sample_t *lsl_outlet_current_sample(lsl_outlet_t *outlet) {
    return (lsl_sample_t *) outlet->sample_buffer;
}

int lsl_outlet_set_channel_value(lsl_outlet_t *outlet, size_t channel, lsl_sample_value_t value) {
    RETURN_ON_ERROR(ASSERT(channel < outlet->config.channel_info.num_channels), "Channel index %zu out of range.", channel);
    lsl_outlet_current_sample(outlet)->values[channel] = value;
    return 0;
}

int lsl_outlet_set_channel_value_float(lsl_outlet_t *outlet, size_t channel, float value) {
    RETURN_ON_ERROR(ASSERT(outlet->config.channel_info.format == LSL_FLOAT32), "Incompatible channel format %d.", outlet->config.channel_info.format);
    return lsl_outlet_set_channel_value(outlet, channel, ((lsl_sample_value_t) { .float32_value = value }));
}

int lsl_outlet_set_channel_value_double(lsl_outlet_t *outlet, size_t channel, double value) {
    RETURN_ON_ERROR(ASSERT(outlet->config.channel_info.format == LSL_DOUBLE64), "Incompatible channel format %d.", outlet->config.channel_info.format);
    return lsl_outlet_set_channel_value(outlet, channel, ((lsl_sample_value_t) { .double64_value = value }));
}

int lsl_outlet_set_channel_value_int32(lsl_outlet_t *outlet, size_t channel, int32_t value) {
    RETURN_ON_ERROR(ASSERT(outlet->config.channel_info.format == LSL_INT32), "Incompatible channel format %d.", outlet->config.channel_info.format);
    return lsl_outlet_set_channel_value(outlet, channel, ((lsl_sample_value_t) { .int32_value = value }));
}

int lsl_outlet_set_channel_value_int16(lsl_outlet_t *outlet, size_t channel, int16_t value) {
    RETURN_ON_ERROR(ASSERT(outlet->config.channel_info.format == LSL_INT16), "Incompatible channel format %d.", outlet->config.channel_info.format);
    return lsl_outlet_set_channel_value(outlet, channel, ((lsl_sample_value_t) { .int16_value = value }));
}

int lsl_outlet_set_channel_value_int8(lsl_outlet_t *outlet, size_t channel, int8_t value) {
    RETURN_ON_ERROR(ASSERT(outlet->config.channel_info.format == LSL_INT8), "Incompatible channel format %d.", outlet->config.channel_info.format);
    return lsl_outlet_set_channel_value(outlet, channel, ((lsl_sample_value_t) { .int8_value = value }));
}

int lsl_outlet_set_channel_value_int64(lsl_outlet_t *outlet, size_t channel, int64_t value) {
    RETURN_ON_ERROR(ASSERT(outlet->config.channel_info.format == LSL_INT64), "Incompatible channel format %d.", outlet->config.channel_info.format);
    return lsl_outlet_set_channel_value(outlet, channel, ((lsl_sample_value_t) { .int64_value = value }));
}
