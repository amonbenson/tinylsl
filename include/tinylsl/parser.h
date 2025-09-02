#pragma once

#include <stdint.h>
#include <stddef.h>

#define LSL_PARSE_EXPR(parser, expr) do { \
    while (((parser)->len) > 0) { \
        if (!(expr)) { \
            break; \
        } \
        (parser)->buf++; \
        (parser)->len--; \
    } \
} while (0)

#define LSL_PARSE_TOKEN(parser, start_expr, end_expr, token, token_len) do { \
    LSL_PARSE_EXPR(parser, start_expr); \
    (token) = (parser)->buf; \
    LSL_PARSE_EXPR(parser, end_expr); \
    (token_len) = (parser)->buf - (token); \
} while(0)

typedef struct {
    const uint8_t *buf;
    size_t len;
} lsl_parser_t;

void lsl_parser_create(lsl_parser_t *parser, const uint8_t *buf, size_t len);

size_t lsl_parse_line(lsl_parser_t *parser, const uint8_t **line);
size_t lsl_parse_word(lsl_parser_t *parser, const uint8_t **word);
int lsl_parse_uint64(lsl_parser_t *parser, uint64_t *value);
