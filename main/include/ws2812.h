#ifndef WS2812_H
#define WS2812_H

#include <stddef.h>
#include <stdint.h>
#include "esp_err.h"
#include "ws2812_port.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} ws2812_rgb_t;

typedef struct {
    uint16_t led_count;
    uint8_t brightness;
    ws2812_port_config_t port_cfg;
} ws2812_config_t;

typedef struct {
    uint16_t led_count;
    uint8_t brightness;
    ws2812_port_t port;
    uint8_t *grb_buf;
    uint8_t *tx_buf;
    size_t tx_buf_len;
} ws2812_t;

esp_err_t ws2812_init(ws2812_t *strip, const ws2812_config_t *cfg);
void ws2812_deinit(ws2812_t *strip);

esp_err_t ws2812_set_pixel_rgb(ws2812_t *strip, uint16_t index, uint8_t r, uint8_t g, uint8_t b);
esp_err_t ws2812_set_pixel(ws2812_t *strip, uint16_t index, ws2812_rgb_t color);
void ws2812_set_brightness(ws2812_t *strip, uint8_t brightness);

esp_err_t ws2812_show(ws2812_t *strip);
esp_err_t ws2812_clear(ws2812_t *strip);

#ifdef __cplusplus
}
#endif

#endif
