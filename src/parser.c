#include "tinylsl/parser.h"

#include "error_macros.h"

#define LSL_PARSE_UNTIL_EXPR(buf, len, expr) do { \
    while ((len) > 0) { \
        if (expr) { \
            break; \
        } \
        (buf)++; \
        (len)--; \
    } \
} while (0)

#define LSL_PARSE_TOKEN(buf, len, start_expr, end_expr, token, token_len) do { \
    LSL_PARSE_UNTIL_EXPR(buf, len, start_expr); \
    (token) = (buf); \
    LSL_PARSE_UNTIL_EXPR(buf, len, end_expr); \
    (token_len) = (buf) - (token); \
} while(0)

size_t lsl_parse_line(uint8_t **buf, size_t *len, const uint8_t **line) {
    uint8_t *token;
    size_t token_len;

    LSL_PARSE_TOKEN(
        *buf,
        *len,
        !(**buf == '\r' || **buf == '\n' || **buf == '\t' || **buf == ' '),
        **buf == '\r' || **buf == '\n',
        token,
        token_len
    );

    return token_len;
}

size_t lsl_parse_word(uint8_t **buf, size_t *len, const uint8_t **word) {
    uint8_t *token;
    size_t token_len;

    LSL_PARSE_TOKEN(
        *buf,
        *len,
        !(**buf == '\r' || **buf == '\n' || **buf == '\t' || **buf == ' '),
        **buf == '\r' || **buf == '\n' || **buf == '\t' || **buf == ' ',
        token,
        token_len
    );

    return token_len;
}

int parse_uint(const uint8_t *buf, size_t len, unsigned int *out) {
    TRY_OR_RETURN(TRY_ASSERT(len > 0 && buf != NULL && out != NULL), "Invalid parameters.");

    unsigned int value = 0;

    while ((len) > 0) {
        TRY_OR_RETURN(TRY_ASSERT(*buf >= '0' && *buf <= '9'), "Number contains invalid digit: '%c'", *buf);
        value = value * 10 + *buf - '0';
    }

    *out = value;

    return 0;
}
