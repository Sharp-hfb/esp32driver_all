#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ws2812.h"
#include "ws2812_app.h"

#define APP_LED_NUM             8
#define APP_WS2812_DATA_GPIO    2
#define APP_SPI_CLOCK_GPIO      4
#define APP_BRIGHTNESS          64

static ws2812_t s_strip;


esp_err_t ws2812_app_start(void)
{
    ws2812_config_t cfg = {
        .led_count = APP_LED_NUM,
        .brightness = APP_BRIGHTNESS,
        .port_cfg = {
            .data_gpio = APP_WS2812_DATA_GPIO,
            .clock_gpio = APP_SPI_CLOCK_GPIO,
            .bit_rate_hz = 2500000,
            .max_transfer_bytes = (APP_LED_NUM * 9) + 24,
        },
    };

    esp_err_t err = ws2812_init(&s_strip, &cfg);
    if (err != ESP_OK) {
        return err;
    }

    ws2812_set_pixel(&s_strip, 0, (ws2812_rgb_t){255, 0, 0});
    ws2812_set_pixel(&s_strip, 1, (ws2812_rgb_t){64, 0, 0});
    ws2812_set_pixel(&s_strip, 2, (ws2812_rgb_t){0, 128, 32});
    ws2812_show(&s_strip);

    return ESP_OK;
}
