#include "tinylsl/types.h"

size_t lsl_integral_sizes[8] = {
    0,
    4,
    8,
    0,
    4,
    2,
    1,
    8,
};

const char *lsl_channel_format_names[8] = {
    "undefined",
    "float32",
    "double64",
    "string",
    "int32",
    "int16",
    "int8",
    "int64",
};
