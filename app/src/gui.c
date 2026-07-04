#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/display.h>
#include <zephyr/devicetree.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <lvgl.h>
#include "gui.h"

LOG_MODULE_REGISTER(gui, LOG_LEVEL_INF);

/* Touch (CST816T) is currently not wired into LVGL input to keep this bring-up stable.
 * We'll add it after the display is proven working.
 */

static void ui_create(void)
{
	lv_obj_t *label = lv_label_create(lv_scr_act());
	lv_label_set_text(label, "Hello ST7789V2 + LVGL");
	lv_obj_center(label);
}

static void display_smoke_test(const struct device *display_dev)
{
	struct display_capabilities caps;
	display_get_capabilities(display_dev, &caps);
	LOG_INF("display caps: %ux%u pixel_format=%d", caps.x_resolution, caps.y_resolution,
		caps.current_pixel_format);

	/* ST7789V2 typically uses RGB565 in this project. */
	if (caps.current_pixel_format != PIXEL_FORMAT_RGB_565 &&
	    caps.current_pixel_format != PIXEL_FORMAT_BGR_565) {
		LOG_WRN("display_smoke_test: unsupported pixel format for RGB565 test");
		return;
	}
	LOG_INF("display pixel format: %s",
		(caps.current_pixel_format == PIXEL_FORMAT_RGB_565) ? "RGB565" : "BGR565");

	/* Now uses RGB565_GREEN from gui.h (already byte-swapped for wire order). */
	const uint16_t test_color = RGB565_YELLOW;
	static uint16_t buf[60 * 60];
	for (size_t i = 0; i < ARRAY_SIZE(buf); i++) {
		buf[i] = test_color;
	}

	int ret = display_set_pixel_format(display_dev, caps.current_pixel_format);
	LOG_INF("display_set_pixel_format ret=%d", ret);

	struct display_buffer_descriptor desc = {
		.buf_size = sizeof(buf),
		.width = 60,
		.height = 60,
		.pitch = 60,
		.frame_incomplete = false,
	};

	/* Draw multiple corners + center to catch x/y-offset or margin mistakes. */
	const uint16_t xs[] = {0, (caps.x_resolution > desc.width) ? (caps.x_resolution - desc.width) : 0};
	const uint16_t ys[] = {0, (caps.y_resolution > desc.height) ? (caps.y_resolution - desc.height) : 0};
	for (size_t yi = 0; yi < ARRAY_SIZE(ys); yi++) {
		for (size_t xi = 0; xi < ARRAY_SIZE(xs); xi++) {
			ret = display_write(display_dev, xs[xi], ys[yi], &desc, buf);
			LOG_INF("display_write(%u,%u) ret=%d", xs[xi], ys[yi], ret);
		}
	}

	/* Center patch as well. */
	ret = display_write(display_dev, (caps.x_resolution - desc.width) / 2,
			     (caps.y_resolution - desc.height) / 2, &desc, buf);
	LOG_INF("display_write(center) ret=%d", ret);
}


static void display_color_bars(const struct device *display_dev)
{
	struct display_capabilities caps;
	display_get_capabilities(display_dev, &caps);
	uint16_t w = caps.x_resolution;
	uint16_t h = caps.y_resolution;

	static const uint16_t colors[] = {
		RGB565_RED,
		RGB565_GREEN,
		RGB565_BLUE,
		RGB565_YELLOW,
		RGB565_CYAN,
		RGB565_MAGENTA,
		RGB565_WHITE,
		RGB565_GRAY,
		RGB565_BLACK,
	};

	uint16_t bar_h = h / 9;
	uint16_t extra = h - bar_h * 9;
	uint16_t buf[240];
	uint16_t y = 0;

	struct display_buffer_descriptor desc = {
		.buf_size = sizeof(buf),
		.width = w,
		.height = 1,
		.pitch = w,
		.frame_incomplete = false,
	};

	for (size_t b = 0; b < 9; b++) {
		for (size_t i = 0; i < w; i++) {
			buf[i] = colors[b];
		}
		uint16_t rows = bar_h + (b == 8 ? extra : 0);
		for (uint16_t r = 0; r < rows; r++) {
			display_write(display_dev, 0, y++, &desc, buf);
		}
	}

	LOG_INF("color bars done (%u rows)", y);
}

static void display_clear(const struct device *display_dev, uint16_t color)
{
	struct display_capabilities caps;
	display_get_capabilities(display_dev, &caps);

	uint16_t buf[240];
	for (size_t i = 0; i < ARRAY_SIZE(buf); i++) {
		buf[i] = color;
	}

	struct display_buffer_descriptor desc = {
		.buf_size = sizeof(buf),
		.width = caps.x_resolution,
		.height = 1,
		.pitch = caps.x_resolution,
		.frame_incomplete = false,
	};

	for (uint16_t y = 0; y < caps.y_resolution; y++) {
		display_write(display_dev, 0, y, &desc, buf);
	}
}

void gui_init(void)
{
	const struct device *display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
	if (!device_is_ready(display_dev)) {
		LOG_ERR("Display device not ready");
		return;
	}

	LOG_INF("gui_init: display ready");
	ui_create();
	LOG_INF("gui_init: ui created");

	/* Backlight: P1.03 (avoid P0.04 photo sensor on Qingfeng 840 base) */
	{
		int ret;
		const struct device *gpio1 = DEVICE_DT_GET(DT_NODELABEL(gpio1));

		if (device_is_ready(gpio1)) {
			ret = gpio_pin_configure(gpio1, 3, GPIO_OUTPUT_ACTIVE);
			if (ret != 0) {
				LOG_ERR("BL gpio_pin_configure failed: %d", ret);
			} else {
				LOG_INF("BL gpio_pin_configure OK");
			}
			ret = gpio_pin_set(gpio1, 3, 1);
			LOG_INF("BL gpio_pin_set(3,1) ret=%d", ret);
		} else {
			LOG_WRN("gpio1 not ready, cannot enable BL");
		}
	}

	/* Unblank panel so the first flush becomes visible */
	{
		int ret = display_blanking_off(display_dev);
		LOG_INF("display_blanking_off ret=%d", ret);
	}

	display_clear(display_dev, 0xFFFF);
	display_smoke_test(display_dev);
	k_sleep(K_MSEC(2000));
	display_color_bars(display_dev);

	LOG_INF("GUI initialized");
}
