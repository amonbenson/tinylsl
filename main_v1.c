#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <limits.h>
#include <inttypes.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>


#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define METHOD_SHORTINFO "LSL:shortinfo"
#define METHOD_FULLINFO "LSL:fullinfo"
#define METHOD_TIMEDATA "LSL:timedata"
#define METHOD_STREAMFEED "LSL:streamfeed"

#define PARSE_UNTIL(str, len, expr) do { \
    while ((len) > 0) { \
        if (expr) { \
            break; \
        } \
        (str)++; \
        (len)--; \
    } \
} while (0)


#define UDP_SERVER_PORT 16571
#define TCP_SERVER_PORT 16596


int str_to_uint16(const uint8_t *s, size_t len, uint16_t *out) {
    if (len == 0 || s == NULL || out == NULL) {
        return 1;
    }

    uint32_t value = 0; // use wider type to check overflow

    for (size_t i = 0; i < len; i++) {
        uint8_t c = s[i];
        if (c < '0' || c > '9') {
            return 1; // invalid character
        }
        uint32_t digit = (uint32_t) (c - '0');

        // check for overflow *before* applying
        if (value > (UINT16_MAX - digit) / 10) {
            return 1; // would overflow uint16_t
        }

        value = value * 10 + digit;
    }

    *out = (uint16_t) value;
    return 0;
}

size_t lsl_parse_line(const uint8_t **str, size_t *len, const uint8_t **line) {
    PARSE_UNTIL(*str, *len, !(**str == '\r' || **str == '\n' || **str == '\t' || **str == ' '));
    *line = *str;
    PARSE_UNTIL(*str, *len, **str == '\r' || **str == '\n');
    return *str - *line;
}

size_t lsl_parse_word(const uint8_t **str, size_t *len, const uint8_t **word) {
    PARSE_UNTIL(*str, *len, !(**str == '\r' || **str == '\n' || **str == '\t' || **str == ' '));
    *word = *str;
    PARSE_UNTIL(*str, *len, **str == '\r' || **str == '\n' || **str == '\t' || **str == ' ');
    return *str - *word;
}

void handle_shortinfo_packet(int fd, uint32_t source_address, uint16_t source_port, const uint8_t *query, size_t query_len, const uint8_t *query_id, size_t query_id_len) {
    uint8_t buffer[1024];
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(source_port);
    address.sin_addr.s_addr = htonl(source_address);

    // TODO: match query string

    int len = snprintf((char *) buffer, sizeof(buffer),
        "%.*s\r\n"
        "<?xml version=\"1.0\"?>\n"
        "<info>\n"
        "\t<name>Tiny_LSL_Outlet</name>\n"
        "\t<type>EEG</type>\n"
        "\t<channel_count>8</channel_count>\n"
        "\t<channel_format>float32</channel_format>\n"
        "\t<source_id>myuid34234</source_id>\n"
        "\t<nominal_srate>500.0</nominal_srate>\n"
        "\t<version>1.1</version>\n"
        "\t<created_at>146235.8145968000</created_at>\n"
        "\t<uid>7e1900a9-c56e-442e-be9b-2a58c5953b84</uid>\n"
        "\t<session_id>default</session_id>\n"
        "\t<hostname>bonn</hostname>\n"
        "\t<v4address></v4address>\n"
        "\t<v4data_port>%u</v4data_port>\n"
        "\t<v4service_port>%u</v4service_port>\n"
        "\t<v6address></v6address>\n"
        "\t<v6data_port>%u</v6data_port>\n"
        "\t<v6service_port>%u</v6service_port>\n"
        "\t<desc />\n"
        "</info>\n",
        (int) query_id_len,
        query_id,
        TCP_SERVER_PORT,
        TCP_SERVER_PORT,
        TCP_SERVER_PORT,
        TCP_SERVER_PORT
    );
    if (len < 0) {
        fprintf(stderr, "failed to construct response string\r\n");
        return;
    }
    if (len >= sizeof(buffer)) {
        fprintf(stderr, "response too large\r\n");
        return;
    }

    // printf("Sending shortinfo response to port %u ...\r\n", source_port);
    // printf("SEND:\r\n%.*s\r\n\r\n\r\n", len, buffer);

    // printf("%.*s", n, buffer);
    if (sendto(fd, buffer, len, 0, (struct sockaddr *) &address, sizeof(struct sockaddr)) < 0) {
        fprintf(stderr, "failed to send response\r\n");
        return;
    }
}

ssize_t pack_samples(uint8_t *buf, size_t len, float samples[], size_t num_channels) {
    size_t required_size = 9 + num_channels * 4;

    // check buffer size
    if (required_size > len) {
        return -1;
    }

    // buf[0] = 0x01; // TAG_DEDUCED_TIMESTAMP

    buf[0] = 0x02;
    buf[1] = 0xc9;
    buf[2] = 0x76;
    buf[3] = 0xbe;
    buf[4] = 0x9f;
    buf[5] = 0x0c;
    buf[6] = 0x24;
    buf[7] = 0xfe;
    buf[8] = 0x40;

    for (size_t i = 0; i < num_channels; i++) {
        uint32_t sample = *((uint32_t *) &samples[i]);
        buf[i * 4 + 9] = (sample >> 0) & 0xff;
        buf[i * 4 + 10] = (sample >> 8) & 0xff;
        buf[i * 4 + 11] = (sample >> 16) & 0xff;
        buf[i * 4 + 12] = (sample >> 24) & 0xff;
    }

    return required_size;
}

void handle_streamfeed_packet(int fd, uint32_t source_address, const uint8_t *params, size_t params_len, int version, const uint8_t *stream_uid) {
    // printf("HANDLE STREAMFED version=%d, stream_uid=%.36s, params=%.*s\r\n", version, stream_uid, (int) params_len, params);

    // TODO: validate version and stream uid

    // send outlet stream parameters
    const char *response = "LSL/110 200 OK\r\n"
        "UID: 7e1900a9-c56e-442e-be9b-2a58c5953b84\r\n"
        "Byte-Order: 1234\r\n"
        "Suppress-Subnormals: 0\r\n"
        "Data-Protocol-Version: 110\r\n"
        "\r\n";
    if (send(fd, response, strlen(response), 0) < 0) {
        fprintf(stderr, "failed to stream header\r\n");
        return;
    }

    // see https://github.com/sccn/liblsl/blob/dev/src/sample.cpp#L189

    uint8_t buffer[1024];
    buffer[0] = 0x01; // TAG_DEDUCED_TIMESTAMP

    // assign test pattern to buffer
    float testpattern[16] = {
        4.0,
        -5.0,
        6.0,
        -7.0,
        8.0,
        -9.0,
        10.0,
        -11.0,
        2.0,
        -3.0,
        4.0,
        -5.0,
        6.0,
        -7.0,
        8.0,
        -9.0
    };

    // TODO: error handling
    size_t p = 0;
    p += pack_samples(&buffer[p], sizeof(buffer) - p, &testpattern[0], 8);
    p += pack_samples(&buffer[p], sizeof(buffer) - p, &testpattern[8], 8);

    for (size_t x = 0; x < p; x++) {
        printf("%02x ", buffer[x]);
    }
    printf("\r\n");

    if (send(fd, &buffer, p, 0) < 0) {
        fprintf(stderr, "failed to stream test pattern\r\n");
        return;
    }

    for (int i = 0; i < 100; i++) {
        buffer[0] = 0x01; // TAG_DEDUCED_TIMESTAMP

        for (int s = 0; s < 8; s++) {
            buffer[s * 4 + 1] = 0x00;
            buffer[s * 4 + 2] = 0x00;
            buffer[s * 4 + 3] = i % 255;
            buffer[s * 4 + 4] = 0x40;
        }

        if (send(fd, &buffer, 8 * 4 + 1, 0) < 0) {
            fprintf(stderr, "failed to stream data\r\n");
            return;
        }
    }
}

void handle_packet(int fd, uint32_t source_address, const uint8_t *str, size_t len) {
    // printf("RECV:\r\n%.*s\r\n\r\n\r\n", (int) len, str);

    // extract the command
    const uint8_t *method;
    size_t method_len = lsl_parse_line(&str, &len, &method);
    if (method_len == 0) {
        fprintf(stderr, "no method received\r\n");
        return;
    }

    // check command
    if (strncmp((const char *) method, METHOD_SHORTINFO, method_len) == 0) {
        // parse query string
        const uint8_t *query;
        size_t query_len = lsl_parse_line(&str, &len, &query);
        if (query_len == 0) {
            fprintf(stderr, "no query received\r\n");
            return;
        }

        // parse port
        const uint8_t *source_port_str;
        size_t source_port_str_len = lsl_parse_word(&str, &len, &source_port_str);
        if (source_port_str_len == 0) {
            fprintf(stderr, "no port received\r\n");
            return;
        }
        uint16_t source_port;
        if (str_to_uint16(source_port_str, source_port_str_len, &source_port) != 0) {
            fprintf(stderr, "failed to parse port number\r\n");
            return;
        }

        // parse query id
        const uint8_t *query_id;
        size_t query_id_len = lsl_parse_line(&str, &len, &query_id);
        if (query_id_len == 0) {
            fprintf(stderr, "no query id received\r\n");
            return;
        }

        handle_shortinfo_packet(fd, source_address, source_port, query, query_len, query_id, query_id_len);
    } else if (strncmp((const char *) method, METHOD_STREAMFEED, method_len) == 0) {
        handle_streamfeed_packet(fd, source_address, str, len, 100, NULL);
    } else if (strncmp((const char *) method, METHOD_STREAMFEED "/", MIN(method_len, strlen(METHOD_STREAMFEED "/"))) == 0) {
        // move to the start of the method parameters
        const size_t method_params_offset = strlen(METHOD_STREAMFEED "/"); // length of the string "LSL:streamfeed/"
        size_t params_len = method_len > method_params_offset ? method_len - method_params_offset : 0;
        const uint8_t *params = method + method_params_offset;

        // parse the version
        uint16_t version = 100;
        const uint8_t *version_str;
        size_t version_str_len = lsl_parse_word(&params, &params_len, &version_str);
        if (str_to_uint16(version_str, version_str_len, &version) != 0) {
            fprintf(stderr, "invalid version received\r\n");
            version = 100;
        }

        // parse the stream uuid
        const uint8_t *stream_uid = NULL;
        size_t stream_uid_len = lsl_parse_word(&params, &params_len, &stream_uid);
        if (stream_uid_len != 36) {
            fprintf(stderr, "invalid stream_uid received\r\n");
            stream_uid = NULL;
        }

        handle_streamfeed_packet(fd, source_address, str, len, (int) version, stream_uid);
    } else {
        printf("got invalid command method: %.*s\r\n", (int) method_len, method);
    }
}

void *udp_server() {
    uint8_t buffer[1024];

    // create a socket
    printf("UDP: Creating socket ...\r\n");
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) {
        fprintf(stderr, "failed to create socket\r\n");
        return NULL;
    }

    // bind the socket
    printf("UDP: Binding to port %u ...\r\n", UDP_SERVER_PORT);
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(UDP_SERVER_PORT);
    address.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(fd, (struct sockaddr *) &address, sizeof(struct sockaddr)) != 0) {
        fprintf(stderr, "failed to bind socket\r\n");
        return NULL;
    }

    ssize_t len;
    struct sockaddr_in source_address;
    socklen_t source_address_len = sizeof(struct sockaddr);
    while (1) {
        if ((len = recvfrom(fd, buffer, sizeof(buffer), 0, (struct sockaddr *) &source_address, &source_address_len)) < 0) {
            continue;
        }

        handle_packet(fd, ntohl(source_address.sin_addr.s_addr), buffer, len);
    }

    close(fd);
    return NULL;
}

void *tcp_server() {
    uint8_t buffer[1024];

    // create a socket
    printf("TCP: Creating socket ...\r\n");
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        fprintf(stderr, "failed to create socket\r\n");
        return NULL;
    }

    // bind the socket
    printf("TCP: Binding to port %u ...\r\n", TCP_SERVER_PORT);
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(TCP_SERVER_PORT);
    address.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(fd, (struct sockaddr *) &address, sizeof(struct sockaddr)) != 0) {
        fprintf(stderr, "failed to bind socket\r\n");
        return NULL;
    }

    // start listening
    printf("TCP: Start listening ...\r\n");
    if (listen(fd, 5) != 0) {
        fprintf(stderr, "failed to start listening\r\n");
        return NULL;
    }

    printf("TCP: Waiting to accept connection ...\r\n");
    int conn_fd;
    struct sockaddr_in conn_addr;
    socklen_t conn_addr_len = sizeof(struct sockaddr);
    while ((conn_fd = accept(fd, (struct sockaddr *) &conn_addr, &conn_addr_len)) > 0) {
        while (1) {
            printf("TCP: Waiting for packet to arrive ...\r\n");
            ssize_t len;
            if ((len = recv(conn_fd, buffer, sizeof(buffer), 0)) < 0) {
                continue;
            }

            handle_packet(conn_fd, ntohl(conn_addr.sin_addr.s_addr), buffer, len);
            break;
        }

        close(conn_fd);
    }

    close(fd);
    return NULL;
}


int main(int argc, const char *argv[]) {
    pthread_t udp_thread, tcp_thread;

    pthread_create(&udp_thread, NULL, udp_server, NULL);
    pthread_create(&tcp_thread, NULL, tcp_server, NULL);

    pthread_join(udp_thread, NULL);
    pthread_join(tcp_thread, NULL);

    return 0;
}
