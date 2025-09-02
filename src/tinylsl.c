#include <limits.h>
#include <string.h>
#include "tinylsl/tinylsl.h"
#include "tinylsl/utils.h"
#include "tinylsl/parser.h"
#include "error_macros.h"

static int lsl_outlet_handle_sample_available(void *ctx, lsl_outlet_t *outlet, lsl_sample_t *sample) {
    lsl_t *lsl = (lsl_t *) ctx;
    (void) lsl;

    return 0;
}

int lsl_create(lsl_t *lsl, const lsl_config_t *config) {
    lsl->config = *config;
    lsl->callbacks = (lsl_callbacks_t) { 0 };

    // create the default outlet
    TRY_OR_RETURN(lsl_outlet_create(&lsl->outlet, &lsl->config.outlet, config->sample_buffer, config->sample_buffer_len), "Failed to create outlet.");
    lsl->outlet.callbacks.sample_available = lsl_outlet_handle_sample_available;

    lsl->udp_fd = -1;
    lsl->udp_local_port = 0;

    lsl->tcp_fd = -1;
    lsl->tcp_local_port = 0;
    lsl->tcp_remote_address = 0;
    lsl->tcp_remote_port = 0;

    return 0;
}

static int lsl_handle_shortinfo(lsl_t *lsl, uint32_t response_address, uint16_t response_port, const uint8_t *query, size_t query_len, const uint8_t *query_id, size_t query_id_len) {
    // construct the payload
    uint8_t response[1024];
    const lsl_outlet_t *outlet = &lsl->outlet;
    int response_len = snprintf(
        (char *) response,
        sizeof(response),
        "%.*s\r\n"
        "<?xml version=\"1.0\"?>"
        "<info>"
        "<name>%s</name>"
        "<type>%s</type>"
        "<channel_count>%zu</channel_count>"
        "<channel_format>%s</channel_format>"
        "<source_id>%s</source_id>"
        "<nominal_srate>%f</nominal_srate>"
        "<version>1.0</version>"
        "<created_at>146235.8145968000</created_at>"
        "<uid>7e1900a9-c56e-442e-be9b-2a58c5953b84</uid>"
        "<session_id>default</session_id>"
        "<hostname>tinylsl</hostname>"
        "<v4address></v4address>"
        "<v4data_port>%u</v4data_port>"
        "<v4service_port>%u</v4service_port>"
        "<v6address></v6address>"
        "<v6data_port>%u</v6data_port>"
        "<v6service_port>%u</v6service_port>"
        "<desc/>"
        "</info>\n",
        (int) query_id_len,
        query_id,
        outlet->config.name,
        outlet->config.type,
        outlet->config.channel_info.num_channels,
        lsl_channel_format_names[outlet->config.channel_info.format],
        outlet->config.source_id,
        outlet->config.nominal_srate,
        // outlet->config.uid, // TODO: convert to string
        lsl->tcp_local_port,
        lsl->tcp_local_port,
        lsl->tcp_local_port,
        lsl->tcp_local_port
    );
    TRY_OR_RETURN(TRY_ASSERT(response_len > 0), "Failed to construct response string.");
    TRY_OR_RETURN(TRY_ASSERT(response_len < sizeof(response)), "Response too large.");

    // send the response
    LOG_DEBUG("LSL UDP SEND:\r\n%.*s\r\n\r\n", (int) response_len, response);
    TRY_OR_RETURN(EMIT(&lsl->callbacks, udp_send, lsl->udp_fd, response, response_len, response_address, response_port), "Failed to send response.");

    return 0;
}

static int lsl_handle_packet(lsl_t *lsl, const uint8_t *buf, size_t len, uint32_t remote_address, uint16_t remote_port) {
    lsl_parser_t parser;
    lsl_parser_create(&parser, buf, len);

    // extract the method
    const uint8_t *method;
    size_t method_len = lsl_parse_line(&parser, &method);
    TRY_OR_RETURN(TRY_ASSERT(method_len > 0), "Failed to parse method.");

    // check command
    if (strncmp((const char *) method, LSL_METHOD_SHORTINFO, method_len) == 0) {
        // parse query string
        const uint8_t *query;
        size_t query_len = lsl_parse_line(&parser, &query);
        TRY_OR_RETURN(TRY_ASSERT(query_len > 0), "Failed to parse query string.");

        // parse source port
        uint64_t port_u64;
        TRY_OR_RETURN(lsl_parse_uint64(&parser, &port_u64), "Failed to parse source port number.");
        TRY_OR_RETURN(TRY_ASSERT(port_u64 > 0 && port_u64 <= UINT16_MAX), "Failed to parse source port number: Out of range.");
        uint16_t source_port = (uint16_t) port_u64;

        // parser query id
        const uint8_t *query_id;
        size_t query_id_len = lsl_parse_line(&parser, &query_id);
        TRY_OR_RETURN(TRY_ASSERT(query_id_len > 0), "Failed to parse query id.");

        TRY_OR_RETURN(lsl_handle_shortinfo(lsl, remote_address, source_port, query, query_len, query_id, query_id_len), "Failed to handle shortinfo message");
    } else {
        LOG_WARN("Got invalid command method: %.*s", (int) method_len, method);
    }

    return 0;
}

int lsl_udp_connect(lsl_t *lsl, int fd, uint16_t local_port) {
    int ret = 0;
    EMIT(&lsl->callbacks, lock);

    TRY_OR_CLEANUP(TRY_ASSERT(lsl->udp_fd == -1), "UDP is already connected.");

    lsl->udp_fd = fd;
    lsl->udp_local_port = local_port;

    ret = 0;
cleanup:
    EMIT(&lsl->callbacks, unlock);
    return ret;
}

int lsl_udp_recv(lsl_t * lsl, int fd, const uint8_t *buf, size_t len, uint32_t remote_address, uint16_t remote_port) {
    int ret = 0;
    EMIT(&lsl->callbacks, lock);

    TRY_OR_CLEANUP(TRY_ASSERT(lsl->udp_fd == fd), "Incorrect file descriptor.");

    LOG_DEBUG("LSL UDP RECV:\r\n%.*s\r\n\r\n", (int) len, buf);
    TRY_OR_CLEANUP(lsl_handle_packet(lsl, buf, len, remote_address, remote_port), "Failed to handle udp packet.");

    ret = 0;
cleanup:
    EMIT(&lsl->callbacks, unlock);
    return ret;
}

int lsl_udp_disconnect(lsl_t *lsl, int fd) {
    int ret = 0;
    EMIT(&lsl->callbacks, lock);

    TRY_OR_CLEANUP(TRY_ASSERT(lsl->udp_fd == fd), "Incorrect file descriptor.");

    lsl->udp_fd = -1;
    lsl->udp_local_port = 0;

    ret = 0;
cleanup:
    EMIT(&lsl->callbacks, unlock);
    return ret;
}

int lsl_tcp_connect(lsl_t *lsl, int fd, uint16_t local_port) {
    int ret = 0;
    EMIT(&lsl->callbacks, lock);

    TRY_OR_CLEANUP(TRY_ASSERT(lsl->tcp_fd == -1), "TCP is already connected.");

    lsl->tcp_fd = fd;
    lsl->tcp_local_port = local_port;

    ret = 0;
cleanup:
    EMIT(&lsl->callbacks, unlock);
    return ret;
}

int lsl_tcp_accept(lsl_t *lsl, int fd, uint32_t remote_address, uint16_t remote_port) {
    int ret = 0;
    EMIT(&lsl->callbacks, lock);

    TRY_OR_CLEANUP(TRY_ASSERT(lsl->tcp_fd == fd), "Incorrect file descriptor.");
    TRY_OR_CLEANUP(TRY_ASSERT(lsl->tcp_remote_address == 0), "TCP connection already accepted. Only a single stream is supported.");

    lsl->tcp_remote_address = remote_address;
    lsl->tcp_remote_port = remote_port;

    ret = 0;
cleanup:
    EMIT(&lsl->callbacks, unlock);
    return ret;
}

int lsl_tcp_recv(lsl_t * lsl, int fd, const uint8_t *buf, size_t len) {
    int ret = 0;
    EMIT(&lsl->callbacks, lock);

    TRY_OR_CLEANUP(TRY_ASSERT(lsl->tcp_fd == fd), "Incorrect file descriptor.");
    TRY_OR_CLEANUP(TRY_ASSERT(lsl->tcp_remote_address != 0), "No connection was accepted yet.");

    ret = 0;
cleanup:
    EMIT(&lsl->callbacks, unlock);
    return ret;
}

int lsl_tcp_disconnect(lsl_t *lsl, int fd) {
    int ret = 0;
    EMIT(&lsl->callbacks, lock);

    TRY_OR_CLEANUP(TRY_ASSERT(lsl->tcp_fd == fd), "Incorrect file descriptor.");

    lsl->tcp_fd = -1;
    lsl->tcp_local_port = 0;
    lsl->tcp_remote_address = 0;
    lsl->tcp_remote_port = 0;

    ret = 0;
cleanup:
    EMIT(&lsl->callbacks, unlock);
    return ret;
}
