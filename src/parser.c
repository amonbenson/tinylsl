#include "tinylsl/parser.h"

#include "macrotools/error.h"

void lsl_parser_create(lsl_parser_t *parser, const uint8_t *buf, size_t len) {
    parser->buf = buf;
    parser->len = len;
}

size_t lsl_parse_line(lsl_parser_t *parser, const uint8_t **line) {
    size_t line_len;

    LSL_PARSE_TOKEN(
        parser,
        *parser->buf == '\r' || *parser->buf == '\n' || *parser->buf == '\t' || *parser->buf == ' ',
        !(*parser->buf == '\r' || *parser->buf == '\n'),
        *line,
        line_len
    );

    return line_len;
}

size_t lsl_parse_word(lsl_parser_t *parser, const uint8_t **word) {
    size_t word_len;

    LSL_PARSE_TOKEN(
        parser,
        *parser->buf == '\r' || *parser->buf == '\n' || *parser->buf == '\t' || *parser->buf == ' ',
        !(*parser->buf == '\r' || *parser->buf == '\n' || *parser->buf == '\t' || *parser->buf == ' '),
        *word,
        word_len
    );

    return word_len;
}

int lsl_parse_uint64(lsl_parser_t *parser, uint64_t *value) {
    const uint8_t *token;
    size_t token_len;

    // parse the number token
    LSL_PARSE_TOKEN(
        parser,
        *parser->buf == '\r' || *parser->buf == '\n' || *parser->buf == '\t' || *parser->buf == ' ',
        !(*parser->buf == '\r' || *parser->buf == '\n' || *parser->buf == '\t' || *parser->buf == ' '),
        token,
        token_len
    );
    TRY_OR_RETURN(TRY_ASSERT(token_len > 0), "Number of length 0.");

    // convert to an actual decimal number
    *value = 0;
    while ((token_len) > 0) {
        TRY_OR_RETURN(TRY_ASSERT(*token >= '0' && *token <= '9'), "Number contains invalid digit: '%c'", *token);
        *value = *value * 10 + *token - '0';
        token++;
        token_len--;
    }

    return 0;
}

int lsl_parser_skip_n(lsl_parser_t *parser, size_t n) {
    TRY_OR_RETURN(TRY_ASSERT(parser->len >= n), "Buffer not large enough.");

    parser->buf += n;
    parser->len -= n;

    return 0;
}
