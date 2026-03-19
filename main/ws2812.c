#include <stdlib.h>
#include <string.h>

#include "esp_heap_caps.h"
#include "ws2812.h"

#define WS2812_BYTES_PER_LED          3
#define WS2812_ENCODED_BYTES_PER_LED  9
#define WS2812_RESET_BYTES            24

static inline uint8_t scale8(uint8_t value, uint8_t brightness)
{
    return (uint8_t)(((uint16_t)value * brightness) / 255U);
}

static void encode_byte_to_3bytes(uint8_t src, uint8_t out[3])
{
    uint32_t code = 0;

    for (int bit = 7; bit >= 0; --bit) {
        code <<= 3;
        code |= (src & (1U << bit)) ? 0x6U : 0x4U;
    }

    out[0] = (uint8_t)(code >> 16);
    out[1] = (uint8_t)(code >> 8);
    out[2] = (uint8_t)code;
}

esp_err_t ws2812_init(ws2812_t *strip, const ws2812_config_t *cfg)
{
    if (!strip || !cfg || cfg->led_count == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    memset(strip, 0, sizeof(*strip));
    strip->led_count = cfg->led_count;
    strip->brightness = cfg->brightness;

    strip->grb_buf = calloc(cfg->led_count, WS2812_BYTES_PER_LED);
    if (!strip->grb_buf) {
        return ESP_ERR_NO_MEM;
    }

    strip->tx_buf_len = ((size_t)cfg->led_count * WS2812_ENCODED_BYTES_PER_LED) + WS2812_RESET_BYTES;
    strip->tx_buf = heap_caps_calloc(1, strip->tx_buf_len, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    if (!strip->tx_buf) {
        free(strip->grb_buf);
        strip->grb_buf = NULL;
        return ESP_ERR_NO_MEM;
    }

    esp_err_t err = ws2812_port_create_espidf_spi(&strip->port, &cfg->port_cfg);
    if (err != ESP_OK) {
        free(strip->tx_buf);
        free(strip->grb_buf);
        strip->tx_buf = NULL;
        strip->grb_buf = NULL;
        return err;
    }

    return ws2812_clear(strip);
}

void ws2812_deinit(ws2812_t *strip)
{
    if (!strip) {
        return;
    }

    if (strip->port.ops && strip->port.ops->deinit) {
        strip->port.ops->deinit(&strip->port);
    }

    if (strip->tx_buf) {
        free(strip->tx_buf);
    }
    if (strip->grb_buf) {
        free(strip->grb_buf);
    }

    memset(strip, 0, sizeof(*strip));
}

esp_err_t ws2812_set_pixel_rgb(ws2812_t *strip, uint16_t index, uint8_t r, uint8_t g, uint8_t b)
{
    if (!strip || index >= strip->led_count) {
        return ESP_ERR_INVALID_ARG;
    }

    size_t base = (size_t)index * WS2812_BYTES_PER_LED;
    strip->grb_buf[base + 0] = g;
    strip->grb_buf[base + 1] = r;
    strip->grb_buf[base + 2] = b;

    return ESP_OK;
}

esp_err_t ws2812_set_pixel(ws2812_t *strip, uint16_t index, ws2812_rgb_t color)
{
    return ws2812_set_pixel_rgb(strip, index, color.r, color.g, color.b);
}

void ws2812_set_brightness(ws2812_t *strip, uint8_t brightness)
{
    if (!strip) {
        return;
    }
    strip->brightness = brightness;
}

esp_err_t ws2812_show(ws2812_t *strip)
{
    if (!strip || !strip->grb_buf || !strip->tx_buf || !strip->port.ops || !strip->port.ops->write) {
        return ESP_ERR_INVALID_STATE;
    }

    uint8_t *dst = strip->tx_buf;
    size_t raw_len = (size_t)strip->led_count * WS2812_BYTES_PER_LED;

    for (size_t i = 0; i < raw_len; ++i) {
        uint8_t scaled = scale8(strip->grb_buf[i], strip->brightness);
        encode_byte_to_3bytes(scaled, dst);
        dst += 3;
    }

    memset(dst, 0x00, WS2812_RESET_BYTES);
    return strip->port.ops->write(&strip->port, strip->tx_buf, strip->tx_buf_len);
}

esp_err_t ws2812_clear(ws2812_t *strip)
{
    if (!strip || !strip->grb_buf) {
        return ESP_ERR_INVALID_STATE;
    }

    memset(strip->grb_buf, 0, (size_t)strip->led_count * WS2812_BYTES_PER_LED);
    return ws2812_show(strip);
}
