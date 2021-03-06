/*
 * Copyright (C) 2011 XiaoMi, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/input.h>
#include <linux/module.h>
#include <linux/spi/spi.h>
#include "ft5x0x_ts.h"

#define FT5X0X_SPI_READ			0x8000

static int ft5x0x_spi_xfer(struct device *dev,
			u16 addr, const u8 *tx_buf, u8 *rx_buf, u8 len)
{
	struct spi_device  *spi = to_spi_device(dev);
	struct spi_transfer xfers[2];
	struct spi_message  msg;

	spi_message_init(&msg);
	memset(xfers, 0, sizeof(xfers));

	/* 16th bit indicate read(1) or write(0) */
	if (tx_buf)
		addr &= ~FT5X0X_SPI_READ;
	else
		addr |= FT5X0X_SPI_READ;
	addr = cpu_to_be16(addr); /* MSB first */

	xfers[0].tx_buf = &addr;
	xfers[0].len    = sizeof(addr);
	spi_message_add_tail(&xfers[0], &msg);

	xfers[1].tx_buf = tx_buf;
	xfers[1].rx_buf = rx_buf;
	xfers[1].len    = len;
	spi_message_add_tail(&xfers[1], &msg);

	return spi_sync(spi, &msg);
}

static int ft5x0x_spi_recv(struct device *dev,
				void *buf, int len)
{
	return ft5x0x_spi_xfer(dev, 0, NULL, buf, len);
}

static int ft5x0x_spi_send(struct device *dev,
				const void *buf, int len)
{
	return ft5x0x_spi_xfer(dev, 0, buf, NULL, len);
}

static int ft5x0x_spi_read(struct device *dev,
				u8 addr, void *buf, u8 len)
{
	return ft5x0x_spi_xfer(dev, addr, NULL, buf, len);
}

static int ft5x0x_spi_write(struct device *dev,
				u8 addr, const void *buf, u8 len)
{
	return ft5x0x_spi_xfer(dev, addr, buf, NULL, len);
}

static const struct ft5x0x_bus_ops ft5x0x_spi_bops = {
	.bustype = BUS_SPI,
	.recv    = ft5x0x_spi_recv,
	.send    = ft5x0x_spi_send,
	.read    = ft5x0x_spi_read,
	.write   = ft5x0x_spi_write,
};

#if defined(CONFIG_PM) && !defined(CONFIG_HAS_EARLYSUSPEND)
static int ft5x0x_spi_suspend(struct device *dev)
{
	return ft5x0x_suspend(dev_get_drvdata(dev));
}

static int ft5x0x_spi_resume(struct device *dev)
{
	return ft5x0x_resume(dev_get_drvdata(dev));
}

static const struct dev_pm_ops ft5x0x_spi_pm_ops = {
	.suspend = ft5x0x_spi_suspend,
	.resume  = ft5x0x_spi_resume,
};
#endif

static int __devinit ft5x0x_spi_probe(struct spi_device *spi)
{
	struct ft5x0x_data *ft5x0x;

	ft5x0x = ft5x0x_probe(&spi->dev, spi->irq, &ft5x0x_spi_bops);
	if (IS_ERR(ft5x0x))
		return PTR_ERR(ft5x0x);

	spi_set_drvdata(spi, ft5x0x);
	return 0;
}

static int __devexit ft5x0x_spi_remove(struct spi_device *spi)
{
	struct ft5x0x_data *ft5x0x = spi_get_drvdata(spi);
	ft5x0x_remove(ft5x0x);
	return 0;
}

static const struct spi_device_id ft5x0x_spi_id[] = {
	{"ft5x0x_spi", 0},
	{/* end list */}
};
MODULE_DEVICE_TABLE(spi, ft5x0x_spi_id);

static struct spi_driver ft5x0x_spi_driver = {
	.probe         = ft5x0x_spi_probe,
	.remove        = __devexit_p(ft5x0x_spi_remove),
	.driver = {
		.name  = "ft5x0x_spi",
		.owner = THIS_MODULE,
#if defined(CONFIG_PM) && !defined(CONFIG_HAS_EARLYSUSPEND)
		.pm    = &ft5x0x_spi_pm_ops,
#endif
	},
	.id_table      = ft5x0x_spi_id,
};

static int __init ft5x0x_spi_init(void)
{
	return spi_register_driver(&ft5x0x_spi_driver);
}
module_init(ft5x0x_spi_init);

static void __exit ft5x0x_spi_exit(void)
{
	spi_unregister_driver(&ft5x0x_spi_driver);
}
module_exit(ft5x0x_spi_exit);

MODULE_ALIAS("spi:ft5x0x_spi");
MODULE_AUTHOR("Xiang Xiao <xiaoxiang@xiaomi.com>");
MODULE_DESCRIPTION("spi driver for ft5x0x touchscreen");
MODULE_LICENSE("GPL");
