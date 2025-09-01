#pragma once

#include "tinylsl/types.h"
#include "tinylsl/utils.h"

typedef struct {
    lsl_channel_format_t format;
    lsl_channel_order_t order;
    size_t num_channels;
} lsl_outlet_config_t;

typedef struct {
    lsl_outlet_config_t config;
    uint8_t *sample_buffer;
} lsl_outlet_t;

int lsl_outlet_create(lsl_outlet_t *outlet, lsl_outlet_config_t *config, uint8_t *sample_buffer, size_t sample_buffer_len);
int lsl_outlet_destroy(lsl_outlet_t *outlet);
