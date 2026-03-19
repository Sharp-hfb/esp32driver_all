#include "esp_check.h"
#include "ws2812_app.h"

void app_main(void)
{
    ESP_ERROR_CHECK(ws2812_app_start());
}
