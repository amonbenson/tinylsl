#pragma once
#include "log.h"


#ifndef ERROR_RETURN_VARIABLE
#define ERROR_RETURN_VARIABLE ret
#endif

#ifndef ERROR_GOTO_LABEL
#define ERROR_GOTO_LABEL cleanup
#endif

#define TRY_OR_RETURN(expr, format, ...) \
    do { \
        int _ret; \
        if ((_ret = (expr)) != 0) { \
            LOG_ERROR("Error %d: " format, _ret, ##__VA_ARGS__); \
            return _ret; \
        } \
    } while (0)

#define TRY_OR_WARN(expr, format, ...) \
    do { \
        int _ret; \
        if ((_ret = (expr)) != 0) { \
            LOG_WARN("Error %d: " format, _ret, ##__VA_ARGS__); \
        } \
    } while (0)

#define TRY_OR_EXIT(expr, format, ...) \
    do { \
        int _ret; \
        if ((_ret = (expr)) != 0) { \
            LOG_ERROR("Error %d: " format, _ret, ##__VA_ARGS__); \
            exit(1); \
        } \
    } while (0)

#define TRY_OR_CLEANUP(expr, format, ...) \
    do { \
        if ((ERROR_RETURN_VARIABLE = (expr)) != 0) { \
            LOG_ERROR("Error %d: " format, ERROR_RETURN_VARIABLE, ##__VA_ARGS__); \
            goto ERROR_GOTO_LABEL; \
        } \
    } while (0)

#define _TRY_ASSERT(expr, ret, ...) ((expr) ? 0 : (ret))
#define TRY_ASSERT(expr, ...) _TRY_ASSERT(expr, ## __VA_ARGS__, 1)
