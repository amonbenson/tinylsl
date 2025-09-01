#pragma once

#include "tinylsl/types.h"
#include "tinylsl/utils.h"
#include "tinylsl/sample.h"

typedef struct lsl_outlet lsl_outlet_t;

typedef int (*lsl_outlet_sample_available_cb_t)(void *ctx, lsl_outlet_t *outlet, lsl_sample_t *sample);

typedef struct {
    void *ctx;
    lsl_outlet_sample_available_cb_t sample_available;
} lsl_outlet_callbacks_t;

typedef struct {
    lsl_channel_info_t channel_info;
    const char *name;
    const char *type;
    const char *source_id;
    double nominal_srate;
    int version;
    double created_at;
    lsl_uuid_t uid;
    const char *hostname;
} lsl_outlet_config_t;

typedef struct lsl_outlet {
    lsl_outlet_config_t config;
    lsl_outlet_callbacks_t callbacks;
    uint8_t *sample_buffer;
} lsl_outlet_t;

int lsl_outlet_create(lsl_outlet_t *outlet, const lsl_outlet_config_t *config, uint8_t *sample_buffer, size_t sample_buffer_len);
int lsl_outlet_destroy(lsl_outlet_t *outlet);

lsl_sample_t *lsl_outlet_current_sample(lsl_outlet_t *outlet);
int lsl_outlet_push_current_sample(lsl_outlet_t *outlet);

int lsl_outlet_set_channel_value(lsl_outlet_t *outlet, size_t channel, lsl_sample_value_t value);
int lsl_outlet_set_channel_value_float(lsl_outlet_t *outlet, size_t channel, float value);
int lsl_outlet_set_channel_value_double(lsl_outlet_t *outlet, size_t channel, double value);
int lsl_outlet_set_channel_value_int32(lsl_outlet_t *outlet, size_t channel, int32_t value);
int lsl_outlet_set_channel_value_int16(lsl_outlet_t *outlet, size_t channel, int16_t value);
int lsl_outlet_set_channel_value_int8(lsl_outlet_t *outlet, size_t channel, int8_t value);
int lsl_outlet_set_channel_value_int64(lsl_outlet_t *outlet, size_t channel, int64_t value);
