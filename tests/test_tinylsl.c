#include <string.h>
#include <stdbool.h>
#include "macrotools/test.h"
#include "macrotools/log.h"
#include "tinylsl/tinylsl.h"


#define MIN(a, b) ((a) < (b) ? (a) : (b))


uint8_t udp_send_buffer[2048];
size_t udp_send_len = 0;

uint8_t tcp_send_buffer[2048];
size_t tcp_send_len = 0;


static struct udp_send_info {
    int fd;
    uint32_t remote_address;
    uint16_t remote_port;
} udp_send_info;

static struct tcp_send_info {
    int fd;
} tcp_send_info;

static int handle_udp_send(void *ctx, int fd, uint8_t *buf, size_t len, uint32_t remote_address, uint16_t remote_port) {
    udp_send_info = (struct udp_send_info) {
        .fd = fd,
        .remote_address = remote_address,
        .remote_port = remote_port,
    };

    // store the bytes to be send
    if (len <= sizeof(udp_send_buffer) - udp_send_len) {
        memcpy(&udp_send_buffer[udp_send_len], buf, len);
        udp_send_len += len;
        return 0;
    } else {
        LOG_ERROR("UDP send buffer overflow!");
        udp_send_len = 0;
        return 1;
    }
}

static int handle_tcp_send(void *ctx, int fd, uint8_t *buf, size_t len) {
    tcp_send_info = (struct tcp_send_info) {
        .fd = fd,
    };

    // store the bytes to be send
    if (len <= sizeof(tcp_send_buffer) - tcp_send_len) {
        memcpy(&tcp_send_buffer[tcp_send_len], buf, len);
        tcp_send_len += len;
        printf("TCP LEN = %zu\r\n", tcp_send_len);
        return 0;
    } else {
        LOG_ERROR("UDP send buffer overflow!");
        tcp_send_len = 0;
        return 1;
    }
}


DESCRIBE(test_tinylsl, "tinylsl") {
    uint8_t sample_buffer[128];
    const lsl_config_t lsl_config = {
        .outlet = {
            .name = "Test Outlet",
            .type = "EEG",
            .channel_info = {
                .num_channels = 8,
                .format = LSL_FLOAT32,
                .order = LSL_LSB_FIRST,
            },
            .uid = { 0 },
            .source_id = "abc123",
            .nominal_srate = 500.0,
        },
        .sample_buffer = sample_buffer,
        .sample_buffer_len = sizeof(sample_buffer),
    };
    lsl_t lsl;

    IT("responds to shortinfo requests") {
        EXPECT_EQ(lsl_create(&lsl, &lsl_config), 0);
        lsl.callbacks.udp_send = handle_udp_send;

        // connect the servers
        EXPECT_EQ(lsl_tcp_connect(&lsl, 1, 16571), 0);
        EXPECT_EQ(lsl_udp_connect(&lsl, 2, 16572), 0);

        // send the request
        const char *req = "LSL:shortinfo\r\nsession_id='default'\r\n16574 12625478265071873937\r\n";
        udp_send_info = (struct udp_send_info) { 0 };
        udp_send_len = 0;
        EXPECT_EQ(lsl_udp_recv(&lsl, 2, (const uint8_t *) req, strlen(req), 0xC0A80002, 16592), 0);

        // check the udp info
        EXPECT_EQ(udp_send_info.fd, 2);
        EXPECT_EQ(udp_send_info.remote_address, 0xC0A80002);
        EXPECT_EQ(udp_send_info.remote_port, 16574); // note: this is the port from the request payload

        const char *expected_response = "12625478265071873937\r\n"
            "<?xml version=\"1.0\"?>"
            "<info>"
            "<name>Test Outlet</name>"
            "<type>EEG</type>"
            "<channel_count>8</channel_count>"
            "<channel_format>float32</channel_format>"
            "<source_id>abc123</source_id>"
            "<nominal_srate>500.000000</nominal_srate>"
            "<version>1.0</version>"
            "<created_at>146235.8145968000</created_at>"
            "<uid>7e1900a9-c56e-442e-be9b-2a58c5953b84</uid>"
            "<session_id>default</session_id>"
            "<hostname>tinylsl</hostname>"
            "<v4address></v4address>"
            "<v4data_port>16571</v4data_port>"
            "<v4service_port>16571</v4service_port>"
            "<v6address></v6address>"
            "<v6data_port>16571</v6data_port>"
            "<v6service_port>16571</v6service_port>"
            "<desc/>"
            "</info>\n";
        EXPECT_STRING_EQ(udp_send_buffer, expected_response, udp_send_len);
    }

    IT("responds to streamfeed requests") {
        EXPECT_EQ(lsl_create(&lsl, &lsl_config), 0);
        lsl.callbacks.tcp_send = handle_tcp_send;

        // connect the servers
        EXPECT_EQ(lsl_tcp_connect(&lsl, 1, 16571), 0);
        EXPECT_EQ(lsl_udp_connect(&lsl, 2, 16572), 0);

        // accept tcp connection
        EXPECT_EQ(lsl_tcp_accept(&lsl, 1, 0xC0A80002, 16591), 0);

        // send the request
        const char *req = "LSL:streamfeed/110 475919ea-7a7e-4b96-88d1-95197d9adb19\r\n"
            "Native-Byte-Order: 1234\r\n"
            "Endian-Performance: 1.24426e+07\r\n"
            "Has-IEEE754-Floats: 1\r\n"
            "Supports-Subnormals: 1\r\n"
            "Value-Size: 4\r\n"
            "Data-Protocol-Version: 110\r\n"
            "Max-Buffer-Length: 1000\r\n"
            "Max-Chunk-Length: 1\r\n"
            "Hostname: bonn\r\n"
            "Source-Id: ddadfd62\r\n"
            "Session-Id: default\r\n"
            "\r\n";
        tcp_send_info = (struct tcp_send_info) { 0 };
        tcp_send_len = 0;
        EXPECT_EQ(lsl_tcp_recv(&lsl, 1, (const uint8_t *) req, strlen(req)), 0);

        // check the tcp info
        EXPECT_EQ(tcp_send_info.fd, 1);

        const char *expected_header = "LSL/110 200 OK\r\n"
            "UID: 7e1900a9-c56e-442e-be9b-2a58c5953b84\r\n"
            "Byte-Order: 1234\r\n"
            "Suppress-Subnormals: 0\r\n"
            "Data-Protocol-Version: 110\r\n";

        // check the received length
        const size_t header_len = MIN(tcp_send_len, strlen(expected_header));
        const size_t sample_len = 1 + 8 + 8 * 4; // 1 byte timestamp type + 8 bytes double timestamp + 8 channels * 4 bytes float data
        EXPECT_UNSIGNED_GE(tcp_send_len, header_len + 2 * sample_len);

        // check header
        EXPECT_STRING_EQ(tcp_send_buffer, expected_header, header_len);

        // check first sample
        uint8_t s0[4];
        EXPECT_EQ(tcp_send_buffer[header_len + 0], 2); // non-deduced timestamp
        // skip timestamp validation for now
        lsl_sample_value_serialize(&((lsl_sample_value_t) { .float32_value = 4.0f }), &lsl.outlet.config.channel_info, s0);
        EXPECT_BUFFER_EQ(&tcp_send_buffer[header_len + 9 + 0 * 4], s0, 4);
        lsl_sample_value_serialize(&((lsl_sample_value_t) { .float32_value = -5.0f }), &lsl.outlet.config.channel_info, s0);
        EXPECT_BUFFER_EQ(&tcp_send_buffer[header_len + 9 + 1 * 4], s0, 4);
        lsl_sample_value_serialize(&((lsl_sample_value_t) { .float32_value = -11.0f }), &lsl.outlet.config.channel_info, s0);
        EXPECT_BUFFER_EQ(&tcp_send_buffer[header_len + 9 + 7 * 4], s0, 4);

        // check second sample
        EXPECT_EQ(tcp_send_buffer[header_len + sample_len + 0], 2); // non-deduced timestamp
        // skip timestamp validation for now
        lsl_sample_value_serialize(&((lsl_sample_value_t) { .float32_value = 2.0f }), &lsl.outlet.config.channel_info, s0);
        EXPECT_BUFFER_EQ(&tcp_send_buffer[header_len + sample_len + 9 + 0 * 4], s0, 4);
        lsl_sample_value_serialize(&((lsl_sample_value_t) { .float32_value = -3.0f }), &lsl.outlet.config.channel_info, s0);
        EXPECT_BUFFER_EQ(&tcp_send_buffer[header_len + sample_len + 9 + 1 * 4], s0, 4);
        lsl_sample_value_serialize(&((lsl_sample_value_t) { .float32_value = -9.0f }), &lsl.outlet.config.channel_info, s0);
        EXPECT_BUFFER_EQ(&tcp_send_buffer[header_len + sample_len + 9 + 7 * 4], s0, 4);
    }

    IT("pushes sample data via the streamfeed") {
        EXPECT_EQ(lsl_create(&lsl, &lsl_config), 0);
        lsl.callbacks.tcp_send = handle_tcp_send;

        // connect the servers
        EXPECT_EQ(lsl_tcp_connect(&lsl, 1, 16571), 0);
        EXPECT_EQ(lsl_udp_connect(&lsl, 2, 16572), 0);

        // accept tcp connection
        EXPECT_EQ(lsl_tcp_accept(&lsl, 1, 0xC0A80002, 16591), 0);

        // send a streamfeed request
        const char *req = "LSL:streamfeed/110 475919ea-7a7e-4b96-88d1-95197d9adb19\r\n"
            "Native-Byte-Order: 1234\r\n"
            "Endian-Performance: 1.24426e+07\r\n"
            "Has-IEEE754-Floats: 1\r\n"
            "Supports-Subnormals: 1\r\n"
            "Value-Size: 4\r\n"
            "Data-Protocol-Version: 110\r\n"
            "Max-Buffer-Length: 1000\r\n"
            "Max-Chunk-Length: 1\r\n"
            "Hostname: bonn\r\n"
            "Source-Id: ddadfd62\r\n"
            "Session-Id: default\r\n"
            "\r\n";
        EXPECT_EQ(lsl_tcp_recv(&lsl, 1, (const uint8_t *) req, strlen(req)), 0);

        // set the sample data
        lsl_outlet_set_channel_value_float(&lsl.outlet, 0, 1.0f);
        lsl_outlet_set_channel_value_float(&lsl.outlet, 1, 2.0f);
        lsl_outlet_set_channel_value_float(&lsl.outlet, 2, 3.0f);
        lsl_outlet_set_channel_value_float(&lsl.outlet, 3, 4.0f);
        lsl_outlet_set_channel_value_float(&lsl.outlet, 4, 5.0f);
        lsl_outlet_set_channel_value_float(&lsl.outlet, 5, 6.0f);
        lsl_outlet_set_channel_value_float(&lsl.outlet, 6, 7.0f);
        lsl_outlet_set_channel_value_float(&lsl.outlet, 7, 8.0f);

        // reset the tcp send buffer and push a sample
        tcp_send_info = (struct tcp_send_info) { 0 };
        tcp_send_len = 0;
        EXPECT_EQ(lsl_push_values(&lsl), 0);

        // check the received length
        const size_t sample_len = 1 + 8 + 8 * 4; // 1 byte timestamp type + 8 bytes double timestamp + 8 channels * 4 bytes float data
        EXPECT_UNSIGNED_GE(tcp_send_len, sample_len);

        // check the sample data
        uint8_t s0[4];
        EXPECT_EQ(tcp_send_buffer[0], 2); // non-deduced timestamp
        // skip timestamp validation for now
        lsl_sample_value_serialize(&((lsl_sample_value_t) { .float32_value = 1.0f }), &lsl.outlet.config.channel_info, s0);
        EXPECT_BUFFER_EQ(&tcp_send_buffer[9 + 0 * 4], s0, 4);
        lsl_sample_value_serialize(&((lsl_sample_value_t) { .float32_value = 2.0f }), &lsl.outlet.config.channel_info, s0);
        EXPECT_BUFFER_EQ(&tcp_send_buffer[9 + 1 * 4], s0, 4);
        lsl_sample_value_serialize(&((lsl_sample_value_t) { .float32_value = 8.0f }), &lsl.outlet.config.channel_info, s0);
        EXPECT_BUFFER_EQ(&tcp_send_buffer[9 + 7 * 4], s0, 4);
    }
}
