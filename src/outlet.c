#include "tinylsl/outlet.h"
#include "error_macros.h"

int lsl_outlet_create(lsl_outlet_t *outlet, lsl_outlet_config_t *config, uint8_t *sample_buffer, size_t sample_buffer_len) {
    // validate configuration
    RETURN_ON_ERROR(ASSERT(config->num_channels > 0), "Invalid number of channels: %lu", config->num_channels);
    RETURN_ON_ERROR(ASSERT(config->format >= 0 && config->format < 8), "Invalid channel format: %d", config->format);
    RETURN_ON_ERROR(ASSERT(config->order == LSL_LSB_FIRST || config->order == LSL_MSB_FIRST), "Invalid byte order: %d", config->order);

    const size_t required_buffer_len = 1 + 8 + config->num_channels * lsl_integral_sizes[config->format];
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
