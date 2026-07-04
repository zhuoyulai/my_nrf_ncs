#pragma once

#include <zephyr/sys/byteorder.h>

/*
 * MIPI DBI SPI driver ignores pixfmt and sends memory contents
 * byte-by-byte.  On little-endian nRF52, 16-bit pixels are stored
 * [low_byte, high_byte]; SPI sends low byte first, but ST7789V
 * expects high byte first (big-endian on the wire).
 *
 * Therefore each pixel value must be byte-swapped before being
 * written to the display.  The macros below do that automatically.
 */

/** Build a byte-swapped RGB565 pixel (r,g,b each 0..255). */
#define RGB565_BE(r, g, b) \
	sys_cpu_to_be16((uint16_t)(((r) >> 3) << 11) | \
			 (uint16_t)(((g) >> 2) << 5)  | \
			 (uint16_t)((b) >> 3))

/* Pre-computed convenience colours (already in wire order). */
#define RGB565_BLACK   0x0000
#define RGB565_WHITE   0xFFFF
#define RGB565_RED     0x00F8   /* sys_cpu_to_be16(0xF800) */
#define RGB565_GREEN   0xE007   /* sys_cpu_to_be16(0x07E0) */
#define RGB565_BLUE    0x1F00   /* sys_cpu_to_be16(0x001F) */
#define RGB565_YELLOW  0xE0FF   /* sys_cpu_to_be16(0xFFE0) */
#define RGB565_CYAN    0xFF07   /* sys_cpu_to_be16(0x07FF) */
#define RGB565_MAGENTA 0x1FF8   /* sys_cpu_to_be16(0xF81F) */
#define RGB565_GRAY    0x1084   /* sys_cpu_to_be16(0x8410) */

void gui_init(void);
