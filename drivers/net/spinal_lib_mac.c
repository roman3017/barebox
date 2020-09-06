// SPDX-License-Identifier: GPL-2.0
/*
 * spinal_mac.c - A MAC ethernet driver for barebox
 *
 * Copyright (c) 2020 roman3017 <rbacik@hotmail.com>
 */

#include <common.h>
#include <driver.h>
#include <malloc.h>
#include <net.h>
#include <init.h>

#define ETH_MAC_ADDR 0x10040000
#define SPINAL_LIB_MAC_FRAME_SIZE_MAX 2000

#define SPINAL_LIB_MAC_CTRL 0x00
#define SPINAL_LIB_MAC_TX   0x10
#define SPINAL_LIB_MAC_TX_AVAILABILITY   0x14
#define SPINAL_LIB_MAC_RX   0x20
#define SPINAL_LIB_MAC_RX_STATS   0x2C

#define SPINAL_LIB_MAC_CTRL_TX_RESET 0x00000001
#define SPINAL_LIB_MAC_CTRL_TX_READY 0x00000002
#define SPINAL_LIB_MAC_CTRL_TX_ALIGN 0x00000004

#define SPINAL_LIB_MAC_CTRL_RX_RESET 0x00000010
#define SPINAL_LIB_MAC_CTRL_RX_PENDING 0x00000020
#define SPINAL_LIB_MAC_CTRL_RX_ALIGN 0x00000040

#define BUFSIZE (SPINAL_LIB_MAC_FRAME_SIZE_MAX >> 2)

struct spinal_mac_priv {
	void __iomem *regs;
	u16 reserved;
	u32 buf[BUFSIZE];
};

static u32 spinal_mac_tx_availability(void __iomem *base){
	return readl(base + SPINAL_LIB_MAC_TX_AVAILABILITY);
}

static u32 spinal_mac_tx_ready(void __iomem *base){
	return readl(base + SPINAL_LIB_MAC_CTRL) & SPINAL_LIB_MAC_CTRL_TX_READY;
}

static void spinal_mac_tx_u32(void __iomem *base, u32 data){
	writel(data, base + SPINAL_LIB_MAC_TX);
}

#if 0
static u32 spinal_mac_rx_stats(void __iomem *base){
	return readl(base + SPINAL_LIB_MAC_RX_STATS);
}
#endif

static u32 spinal_mac_rx_pending(void __iomem *base){
	return readl(base + SPINAL_LIB_MAC_CTRL) & SPINAL_LIB_MAC_CTRL_RX_PENDING;
}

static u32 spinal_mac_rx_u32(void __iomem *base){
	return readl(base + SPINAL_LIB_MAC_RX);
}

static void spinal_mac_reset_set(void __iomem *base){
	writel(SPINAL_LIB_MAC_CTRL_TX_RESET | SPINAL_LIB_MAC_CTRL_RX_RESET | SPINAL_LIB_MAC_CTRL_TX_ALIGN | SPINAL_LIB_MAC_CTRL_RX_ALIGN, base + SPINAL_LIB_MAC_CTRL);
}

static void spinal_mac_reset_clear(void __iomem *base){
	writel(SPINAL_LIB_MAC_CTRL_TX_ALIGN | SPINAL_LIB_MAC_CTRL_RX_ALIGN, base + SPINAL_LIB_MAC_CTRL);
}

static int spinal_mac_start(struct eth_device *dev)
{
	return 0;
}

static int spinal_mac_send(struct eth_device *dev, void *packet, int length)
{
	struct spinal_mac_priv *priv = dev->priv;
	void __iomem *base = (void *)priv->regs;
	u32 bits = length*8+16;
	u32 word_count = (bits+31)/32;
	u32 *ptr;

	while(!spinal_mac_tx_ready(base));
	spinal_mac_tx_u32(base, bits);
	ptr = (u32*)((u32)packet-2);

	while(word_count){
		u32 tockens = spinal_mac_tx_availability(base);
		if(tockens > word_count) tockens = word_count;
		word_count -= tockens;
		while(tockens--) spinal_mac_tx_u32(base, *ptr++);
	}

	return 0;
}

static int spinal_mac_recv(struct eth_device *dev)
{
	struct spinal_mac_priv *priv = dev->priv;
	void __iomem *base = (void *)priv->regs;

	if (!spinal_mac_rx_pending(base)) return 0;

	u32 bits = spinal_mac_rx_u32(base);
	u32 len = (bits+7)/8-2;
	u32 word_count = (bits+31)/32;
	u32 *ptr;
	uchar *packetp;

	if (word_count > BUFSIZE) {
			debug("%s:%d word_count %d > buf %d\n",__func__,__LINE__,word_count,BUFSIZE);
			while(word_count--) spinal_mac_rx_u32(base);
			return -EMSGSIZE;
	}

	packetp = (uchar *)priv->buf;
	ptr = (u32*)((u32)(packetp)-2);
	while(word_count--){
			*ptr++ = spinal_mac_rx_u32(base);
	}

	/* received packet is padded with two null bytes */
	net_receive(dev, packetp, len);

	return 0;
}

static void spinal_mac_stop(struct eth_device *dev)
{
}

static int spinal_mac_init(struct eth_device *dev)
{
	struct spinal_mac_priv *pdata = dev->priv;
	void __iomem *base = (void *)pdata->regs;
	spinal_mac_reset_set((void *)base);
	udelay(10);
	spinal_mac_reset_clear((void *)base);
	return 0;
}
#if 0
static int eth_open(struct eth_device *edev)
{
	return 0;
}

static void eth_halt(struct eth_device *edev)
{
	/* nothing to do here */
}
#endif
static int get_ethaddr(struct eth_device *edev, unsigned char *adr)
{
	return -1;
}

static int set_ethaddr(struct eth_device *edev, const unsigned char *adr)
{
	return 0;
}

static int spinal_mac_probe(struct device_d *dev)
{
	struct eth_device *edev;
	struct spinal_mac_priv *priv;

	priv = xzalloc(sizeof(struct spinal_mac_priv));
	edev = xzalloc(sizeof(struct eth_device));
	edev->priv = priv;
	edev->parent = dev;

	priv->regs = dev_get_mem_region(dev, 0);
	if (IS_ERR(priv->regs))
		return PTR_ERR(priv->regs);

	edev->init = spinal_mac_init;
	edev->open = spinal_mac_start;
	edev->send = spinal_mac_send;
	edev->recv = spinal_mac_recv;
	edev->halt = spinal_mac_stop;
	edev->get_ethaddr = get_ethaddr;
	edev->set_ethaddr = set_ethaddr;

	dev_info(dev, "spinal_mac_probe %x\n", priv->regs);

	return eth_register(edev);
}

static __maybe_unused struct of_device_id mac_dt_ids[] = {
	{ .compatible = "spinal,lib_mac" },
	{ /* sentinel */ }
};

static struct driver_d spinal_mac_driver = {
	.name  = "spinal_mac_driver",
	.probe = spinal_mac_probe,
	.of_compatible = DRV_OF_COMPAT(mac_dt_ids),
};
device_platform_driver(spinal_mac_driver);
