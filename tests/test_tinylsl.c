#include <string.h>
#include "macrotools/test.h"
#include "macrotools/log.h"
#include "tinylsl/tinylsl.h"


static struct udp_send_info {
    int fd;
    uint8_t *buf;
    size_t len;
    uint32_t remote_address;
    uint16_t remote_port;
} udp_send_info;

static struct tcp_send_info {
    int fd;
    uint8_t *buf;
    size_t len;
} tcp_send_info;

static int handle_udp_send(void *ctx, int fd, uint8_t *buf, size_t len, uint32_t remote_address, uint16_t remote_port) {
    udp_send_info = (struct udp_send_info) {
        .fd = fd,
        .buf = buf,
        .len = len,
        .remote_address = remote_address,
        .remote_port = remote_port,
    };

    return 0;
}

static int handle_tcp_send(void *ctx, int fd, uint8_t *buf, size_t len) {
    tcp_send_info = (struct tcp_send_info) {
        .fd = fd,
        .buf = buf,
        .len = len,
    };

    return 0;
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
        EXPECT_EQ(lsl_udp_recv(&lsl, 2, (const uint8_t *) req, strlen(req), 0xC0A80002, 16592), 0);

        // check the response
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
        EXPECT_EQ(strncmp((const char *) udp_send_info.buf, expected_response, udp_send_info.len), 0);
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
        EXPECT_EQ(lsl_tcp_recv(&lsl, 1, (const uint8_t *) req, strlen(req)), 0);

        // check the response
        EXPECT_EQ(tcp_send_info.fd, 1);

        // const char *expected_response = "12625478265071873937\r\n"
        //     "<?xml version=\"1.0\"?>"
        //     "<info>"
        //     "<name>Test Outlet</name>"
        //     "<type>EEG</type>"
        //     "<channel_count>8</channel_count>"
        //     "<channel_format>float32</channel_format>"
        //     "<source_id>abc123</source_id>"
        //     "<nominal_srate>500.000000</nominal_srate>"
        //     "<version>1.0</version>"
        //     "<created_at>146235.8145968000</created_at>"
        //     "<uid>7e1900a9-c56e-442e-be9b-2a58c5953b84</uid>"
        //     "<session_id>default</session_id>"
        //     "<hostname>tinylsl</hostname>"
        //     "<v4address></v4address>"
        //     "<v4data_port>16571</v4data_port>"
        //     "<v4service_port>16571</v4service_port>"
        //     "<v6address></v6address>"
        //     "<v6data_port>16571</v6data_port>"
        //     "<v6service_port>16571</v6service_port>"
        //     "<desc/>"
        //     "</info>\n";
        // EXPECT_EQ(strncmp((const char *) udp_send_info.buf, expected_response, udp_send_info.len), 0);
    }
}
