#pragma once

#include "tinylsl/outlet.h"
#include "tinylsl/net.h"

#define LSL_METHOD_SHORTINFO "LSL:shortinfo"
#define LSL_METHOD_FULLINFO "LSL:fullinfo"
#define LSL_METHOD_TIMEDATA "LSL:timedata"
#define LSL_METHOD_STREAMFEED "LSL:streamfeed"

typedef struct {
    lsl_outlet_config_t outlet;
    uint8_t *sample_buffer;
    size_t sample_buffer_len;
} lsl_config_t;

typedef void (*lsl_lock_cb)(void *ctx);
typedef void (*lsl_unlock_cb)(void *ctx);
typedef int (*lsl_udp_send_cb)(void *ctx, int fd, uint8_t *buf, size_t len, uint32_t remote_address, uint16_t remote_port);
typedef int (*lsl_tcp_send_cb)(void *ctx, int fd, uint8_t *buf, size_t len);

typedef struct {
    void *ctx;
    lsl_lock_cb lock;
    lsl_unlock_cb unlock;
    lsl_udp_send_cb udp_send;
    lsl_tcp_send_cb tcp_send;
} lsl_callbacks_t;

typedef struct {
    lsl_config_t config;
    lsl_callbacks_t callbacks;

    lsl_outlet_t outlet;

    int udp_fd;
    uint16_t udp_local_port;

    int tcp_fd;
    uint16_t tcp_local_port;
    uint32_t tcp_remote_address;
    uint16_t tcp_remote_port;
} lsl_t;

int lsl_create(lsl_t *lsl, const lsl_config_t *config);
int lsl_start_stream(lsl_t *lsl, net_handle_t fd);
int lsl_stop_stream(lsl_t *lsl);

int lsl_udp_connect(lsl_t *lsl, int fd, uint16_t local_port);
int lsl_udp_recv(lsl_t * lsl, int fd, const uint8_t *buf, size_t len, uint32_t remote_address, uint16_t remote_port);
int lsl_udp_disconnect(lsl_t *lsl, int fd);

int lsl_tcp_connect(lsl_t *lsl, int fd, uint16_t local_port);
int lsl_tcp_accept(lsl_t *lsl, int fd, uint32_t remote_address, uint16_t remote_port);
int lsl_tcp_recv(lsl_t * lsl, int fd, const uint8_t *buf, size_t len);
int lsl_tcp_disconnect(lsl_t *lsl, int fd);
