#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "tinylsl/types.h"

#define LSL_DEDUCED_TIMESTAMP (-1.0 / 0.0)

typedef union {
    float float32_value;
    double double64_value;
    int32_t int32_value;
    int16_t int16_value;
    int8_t int8_value;
    int64_t int64_value;
} lsl_sample_value_t;

typedef struct {
    double timestamp;
    lsl_sample_value_t values[];
} lsl_sample_t;


int lsl_sample_value_serialize(const lsl_sample_value_t *value, const lsl_channel_info_t *channel_info, uint8_t *buf);
int lsl_sample_value_assign_test_pattern(lsl_sample_value_t *sample_value, const lsl_channel_info_t *channel_info, long offset, bool invert);

size_t lsl_sample_serialized_length(const lsl_sample_t *sample, const lsl_channel_info_t *channel_info);
int lsl_sample_serialize(const lsl_sample_t *sample, const lsl_channel_info_t *channel_info, uint8_t *buf, size_t len);
int lsl_sample_assign_test_pattern(lsl_sample_t *sample, const lsl_channel_info_t *channel_info, long offset);
