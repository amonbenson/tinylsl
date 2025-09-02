#include <string.h>
#include "test_macros.h"
#include "tinylsl/parser.h"


DESCRIBE(test_parser, "parser") {
    IT("parsers lines") {
        const char *str = "\n the first line\n   the second line\r\n\tthe third line\n   \n";
        size_t len = strlen(str);

        lsl_parser_t parser;
        lsl_parser_create(&parser, (uint8_t *) str, len);

        const uint8_t *line;
        size_t line_len;

        line_len = lsl_parse_line(&parser, &line);
        EXPECT_EQ(line_len, 14);
        EXPECT_EQ(strncmp((const char *) line, "the first line", line_len), 0);

        line_len = lsl_parse_line(&parser, &line);
        EXPECT_EQ(line_len, 15);
        EXPECT_EQ(strncmp((const char *) line, "the second line", line_len), 0);

        line_len = lsl_parse_line(&parser, &line);
        EXPECT_EQ(line_len, 14);
        EXPECT_EQ(strncmp((const char *) line, "the third line", line_len), 0);

        // end of text, last line should be stripped completely
        line_len = lsl_parse_line(&parser, &line);
        EXPECT_EQ(line_len, 0);

        line_len = lsl_parse_line(&parser, &line);
        EXPECT_EQ(line_len, 0);

        EXPECT_EQ(parser.len, 0);
    }

    IT("parsers words") {
        const char *str = "\n one two\t   three\r\n   four\n ";
        size_t len = strlen(str);

        lsl_parser_t parser;
        lsl_parser_create(&parser, (uint8_t *) str, len);

        const uint8_t *line;
        size_t line_len;

        line_len = lsl_parse_word(&parser, &line);
        EXPECT_EQ(line_len, 3);
        EXPECT_EQ(strncmp((const char *) line, "one", line_len), 0);

        line_len = lsl_parse_word(&parser, &line);
        EXPECT_EQ(line_len, 3);
        EXPECT_EQ(strncmp((const char *) line, "two", line_len), 0);

        line_len = lsl_parse_word(&parser, &line);
        EXPECT_EQ(line_len, 5);
        EXPECT_EQ(strncmp((const char *) line, "three", line_len), 0);

        line_len = lsl_parse_word(&parser, &line);
        EXPECT_EQ(line_len, 4);
        EXPECT_EQ(strncmp((const char *) line, "four", line_len), 0);

        line_len = lsl_parse_word(&parser, &line);
        EXPECT_EQ(line_len, 0);

        EXPECT_EQ(parser.len, 0);
    }

    IT("parses unsigned integers") {
        const char *str = "\n 123\t   0\r\n   x25\n ";
        size_t len = strlen(str);

        lsl_parser_t parser;
        lsl_parser_create(&parser, (uint8_t *) str, len);

        uint64_t value;

        EXPECT_EQ(lsl_parse_uint64(&parser, &value), 0);
        EXPECT_EQ(value, 123);

        EXPECT_EQ(lsl_parse_uint64(&parser, &value), 0);
        EXPECT_EQ(value, 0);

        // should consume the "x25" string but then fail because of the invalid character
        EXPECT_NE(lsl_parse_uint64(&parser, &value), 0);

        // should consume the rest of the input and fail because no token is left
        EXPECT_NE(lsl_parse_uint64(&parser, &value), 0);

        EXPECT_EQ(parser.len, 0);
    }
}
