#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/gpio.h>

#define MY_SPI_MASTER DT_NODELABEL(my_spi_master)

const struct device *spi_dev;
static struct spi_cs_control spim_cs = {
    .gpio = SPI_CS_GPIOS_DT_SPEC_GET(DT_NODELABEL(my_spi_master)),
    .delay = 0,
};

static const struct spi_config spi_cfg = {
    .operation = SPI_WORD_SET(8) | SPI_TRANSFER_MSB,
    .frequency = 4000000,
    .slave = 0,
    .cs = &spim_cs
};

void spi_init(void)
{
    spi_dev = DEVICE_DT_GET(MY_SPI_MASTER);
    if (!device_is_ready(spi_dev)) {
        printk("SPI master device not ready!\n");
    }
    if (!device_is_ready(spim_cs.gpio.port)) {
        printk("SPI master chip select device not ready!\n");
    }
}