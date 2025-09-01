#include <stdint.h>
#include <stddef.h>


typedef int net_handle_t;

typedef enum {
    NET_UDP = 0,
    NET_TCP = 1,
} net_protocol_t;

typedef net_handle_t (*net_connect_cb)(net_protocol_t protocol);
typedef int (*net_recv(net_handle_t fd, uint8_t *buf, size_t len));
typedef int (*net_recvfrom(net_handle_t fd, uint8_t *buf, size_t len, uint32_t *source_address, uint16_t *source_port));
typedef int (*net_send(net_handle_t fd, uint8_t *buf, size_t len));
typedef int (*net_sendto(net_handle_t fd, uint8_t *buf, size_t len, uint32_t *source_address, uint16_t *source_port));

typedef struct {
    net_connect_cb create_socket;
} net_callbacks_t;
