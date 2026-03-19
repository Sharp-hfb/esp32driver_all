# ESP-IDF WS2812 Portable SPI Example

本工程使用 **SPI 位编码** 驱动 WS2812 / WS2812B，不依赖乐鑫自带 `led_strip` 灯带库。

## 分层结构

- `ws2812_port.h` + `ws2812_port_espidf_spi.c`
  - 平台适配层，只负责初始化外设、发送编码后的字节流、提供微秒延时
- `ws2812.h` + `ws2812.c`
  - 协议/驱动层，负责颜色缓存、亮度缩放、GRB 排列、WS2812 位编码
- `ws2812_app.h` + `ws2812_app.c`
  - 应用层，负责动画效果
- `main.c`
  - 入口

## 原理

SPI 时钟设置为 **2.5 MHz**：

- 1 个 SPI bit = 0.4 us
- WS2812 的每 1bit 数据编码为 3bit SPI：
  - `0 -> 100`  => 高 0.4 us，低 0.8 us
  - `1 -> 110`  => 高 0.8 us，低 0.4 us

这样可以逼近 WS2812B 典型时序窗口（总周期约 1.25 us，GRB 顺序，高位先发）。

## 接线

- `data_gpio` 接 WS2812 的 `DIN`
- `clock_gpio` 只给 SPI 外设占用，**不要接灯带**
- 灯带供电按实际电流预留，注意共地
- 推荐使用 5V 灯带时加电平转换

## 默认配置

在 `main/ws2812_app.c` 里修改：

- `APP_LED_NUM`
- `APP_WS2812_DATA_GPIO`
- `APP_SPI_CLOCK_GPIO`
- `APP_BRIGHTNESS`

## 编译

```bash
idf.py set-target esp32
idf.py build
```

或者替换为你的目标芯片，例如：

```bash
idf.py set-target esp32s3
idf.py build flash monitor
```

## 迁移到其他平台

保留：

- `ws2812.c`
- `ws2812.h`
- `ws2812_app.c`
- `ws2812_app.h`

只需要重写：

- `ws2812_port_*.c`

并实现这 4 个动作：

- `init()`
- `write()`
- `delay_us()`
- `deinit()`
