#pragma once

#include <stdint.h>
#include <stddef.h>

typedef enum {
    LSL_UNDEFINED = 0,
    LSL_FLOAT32 = 1,
    LSL_DOUBLE64 = 2,
    LSL_STRING = 3,
    LSL_INT32 = 4,
    LSL_INT16 = 5,
    LSL_INT8 = 6,
    LSL_INT64 = 7,
} lsl_channel_format_t;

typedef enum {
    LSL_LSB_FIRST = 0,
    LSL_MSB_FIRST = 1,
} lsl_channel_order_t;

extern size_t lsl_integral_sizes[8];

typedef union {
    float value_float32;
    double value_double64;
    int32_t value_int32;
    int16_t value_int16;
    int8_t value_int8;
    int64_t value_int64;
} lsl_integral_value_t;
