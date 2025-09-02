#include "tinylsl/tinylsl.h"
#include "error_macros.h"
#include "tinylsl/utils.h"

static int lsl_outlet_handle_sample_available(void *ctx, lsl_outlet_t *outlet, lsl_sample_t *sample) {
    lsl_t *lsl = (lsl_t *) ctx;
    (void) lsl;

    return 0;
}

int lsl_create(lsl_t *lsl, const lsl_config_t *config, uint8_t *sample_buffer, size_t sample_buffer_len) {
    lsl->config = *config;

    // create the default outlet
    TRY_OR_RETURN(lsl_outlet_create(&lsl->outlet, &lsl->config.outlet, sample_buffer, sample_buffer_len), "Failed to create outlet.");
    lsl->outlet.callbacks.sample_available = lsl_outlet_handle_sample_available;

    lsl->udp_fd = -1;
    lsl->udp_local_port = 0;

    lsl->tcp_fd = -1;
    lsl->tcp_local_port = 0;
    lsl->tcp_remote_address = 0;
    lsl->tcp_remote_port = 0;

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
