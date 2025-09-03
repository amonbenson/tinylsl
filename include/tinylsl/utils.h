#pragma once

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define EMIT_DEFAULT(callbacks, name, default, ...) (((callbacks)->name) \
    ? ((callbacks)->name)((callbacks)->ctx, ##__VA_ARGS__) \
    : (default))
#define EMIT(callbacks, name, ...) EMIT_DEFAULT(callbacks, name, 0, ##__VA_ARGS__)
