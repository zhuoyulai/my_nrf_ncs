#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/gpio.h>
#include "com_spi.h"

#define MY_SPI_MASTER DT_NODELABEL(my_spi_master)

const struct device *g_spi_dev = DEVICE_DT_GET(MY_SPI_MASTER);
const struct device *gpio_dev = DEVICE_DT_GET(DT_NODELABEL(gpio0));
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

struct spi_dt_spec spispec = {
    .bus = NULL,
    .config = spi_cfg,
};

int m_spi_init(struct com_spi_dev_t *dev, const spi_config_t *cfg)
{
    int ret = 0;
    spispec.bus = g_spi_dev;
    if (!device_is_ready(g_spi_dev)) {
        printk("SPI master device not ready!\n");
        return -ENODEV;
    }
    if (!device_is_ready(gpio_dev)) {
        printk("SPI master chip select device not ready!\n");
        return -ENODEV;
    }
    return ret;
}

int m_spi_write_data(struct com_spi_dev_t *dev, const uint8_t *tx_buf, size_t len)
{
    struct spi_buf buf_t = { .buf = tx_buf, .len = len };
    struct spi_buf_set tx_set = { .buffers = &buf_t, .count = 1 };
    return spi_transceive_dt(&spispec, &tx_set, NULL);
}

int m_spi_read_data(struct com_spi_dev_t *dev, uint8_t *rx_buf, size_t len)
{
    struct spi_buf buf_r = { .buf = rx_buf, .len = len };
    struct spi_buf_set rx_set = { .buffers = &buf_r, .count = 1 };
    return spi_transceive_dt(&spispec, NULL, &rx_set);
}

int m_spi_instance_init(void)
{
    int ret = 0;
    spi_dev_instance_t *dev = com_spi_get_instance();
    if (!dev) {
        return -ENODEV;
    }
    
    dev->id = 0;
    dev->config.freq_hz = 1000000;
    dev->config.mode = SPI_MODE0;
    dev->config.bit_order = 0;
    dev->priv = NULL;
    dev->ops->write = m_spi_write_data;
    dev->ops->read = m_spi_read_data;
    dev->ops->init = m_spi_init;
    dev->ops->deinit = NULL;
    dev->ops->transfer = NULL;

    if(dev->ops->init)
        ret = dev->ops->init(NULL,NULL);

    if (ret < 0) {
        assert_print(ret);
    }
    return ret;
}