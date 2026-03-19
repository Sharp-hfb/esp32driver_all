#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "driver/spi_master.h"
#include "esp_err.h"
#include "esp_rom_sys.h"
#include "ws2812_port.h"

typedef struct {
    spi_device_handle_t spi;
    spi_host_device_t host;
    bool bus_inited;
    bool dev_inited;
} ws2812_port_espidf_spi_ctx_t;

static esp_err_t port_spi_init(ws2812_port_t *port, const ws2812_port_config_t *cfg)
{
    if (!port || !cfg) {
        return ESP_ERR_INVALID_ARG;
    }

    ws2812_port_espidf_spi_ctx_t *ctx = calloc(1, sizeof(*ctx));
    if (!ctx) {
        return ESP_ERR_NO_MEM;
    }

    ctx->host = SPI2_HOST;

    spi_bus_config_t bus_cfg = {
        .mosi_io_num = cfg->data_gpio,
        .miso_io_num = -1,
        .sclk_io_num = cfg->clock_gpio,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = (int)cfg->max_transfer_bytes,
    };

    esp_err_t err = spi_bus_initialize(ctx->host, &bus_cfg, SPI_DMA_CH_AUTO);
    if (err != ESP_OK) {
        free(ctx);
        return err;
    }
    ctx->bus_inited = true;

    spi_device_interface_config_t dev_cfg = {
        .clock_speed_hz = (int)cfg->bit_rate_hz,
        .mode = 0,
        .spics_io_num = -1,
        .queue_size = 1,
        .command_bits = 0,
        .address_bits = 0,
        .dummy_bits = 0,
    };

    err = spi_bus_add_device(ctx->host, &dev_cfg, &ctx->spi);
    if (err != ESP_OK) {
        spi_bus_free(ctx->host);
        free(ctx);
        return err;
    }
    ctx->dev_inited = true;

    port->ctx = ctx;
    return ESP_OK;
}

static esp_err_t port_spi_write(ws2812_port_t *port, const uint8_t *data, size_t len)
{
    if (!port || !port->ctx || !data || len == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    ws2812_port_espidf_spi_ctx_t *ctx = (ws2812_port_espidf_spi_ctx_t *)port->ctx;

    spi_transaction_t t = {0};
    t.length = (uint32_t)(len * 8U);
    t.tx_buffer = data;

    return spi_device_transmit(ctx->spi, &t);
}

static void port_spi_delay_us(uint32_t us)
{
    esp_rom_delay_us(us);
}

static void port_spi_deinit(ws2812_port_t *port)
{
    if (!port || !port->ctx) {
        return;
    }

    ws2812_port_espidf_spi_ctx_t *ctx = (ws2812_port_espidf_spi_ctx_t *)port->ctx;

    if (ctx->dev_inited) {
        spi_bus_remove_device(ctx->spi);
    }
    if (ctx->bus_inited) {
        spi_bus_free(ctx->host);
    }

    free(ctx);
    port->ctx = NULL;
}

static const ws2812_port_ops_t s_port_ops = {
    .init = port_spi_init,
    .write = port_spi_write,
    .delay_us = port_spi_delay_us,
    .deinit = port_spi_deinit,
};

esp_err_t ws2812_port_create_espidf_spi(ws2812_port_t *port, const ws2812_port_config_t *cfg)
{
    if (!port || !cfg) {
        return ESP_ERR_INVALID_ARG;
    }

    memset(port, 0, sizeof(*port));
    port->ops = &s_port_ops;
    return port->ops->init(port, cfg);
}
