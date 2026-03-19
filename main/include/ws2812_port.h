#ifndef WS2812_PORT_H
#define WS2812_PORT_H

#include <stddef.h>
#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ws2812_port ws2812_port_t;

typedef struct {
    int data_gpio;
    int clock_gpio;
    uint32_t bit_rate_hz;
    size_t max_transfer_bytes;
} ws2812_port_config_t;

typedef struct {
    esp_err_t (*init)(ws2812_port_t *port, const ws2812_port_config_t *cfg);
    esp_err_t (*write)(ws2812_port_t *port, const uint8_t *data, size_t len);
    void (*delay_us)(uint32_t us);
    void (*deinit)(ws2812_port_t *port);
} ws2812_port_ops_t;

struct ws2812_port {
    const ws2812_port_ops_t *ops;
    void *ctx;
};

esp_err_t ws2812_port_create_espidf_spi(ws2812_port_t *port, const ws2812_port_config_t *cfg);

#ifdef __cplusplus
}
#endif

#endif
