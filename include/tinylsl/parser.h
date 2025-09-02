#pragma once

#include <stdint.h>
#include <stddef.h>

size_t lsl_parse_line(uint8_t **buf, size_t *len, const uint8_t **line);
size_t lsl_parse_word(uint8_t **buf, size_t *len, const uint8_t **word);

int parse_uint(const uint8_t *buf, size_t len, unsigned int *out);
