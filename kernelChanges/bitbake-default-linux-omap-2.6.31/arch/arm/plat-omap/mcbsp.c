/*
 * linux/arch/arm/plat-omap/mcbsp.c
 *
 * Copyright (C) 2004 Nokia Corporation
 * Author: Samuel Ortiz <samuel.ortiz@nokia.com>
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Multichannel mode not supported.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/wait.h>
#include <linux/completion.h>
#include <linux/interrupt.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/io.h>

#include <mach/dma.h>
#include <mach/mcbsp.h>

struct omap_mcbsp **mcbsp_ptr;
int omap_mcbsp_count;

void omap_mcbsp_write(void __iomem *io_base, u16 reg, u32 val)
{
	if (cpu_class_is_omap1() || cpu_is_omap2420())
		__raw_writew((u16)val, io_base + reg);
	else
		__raw_writel(val, io_base + reg);
}

int omap_mcbsp_read(void __iomem *io_base, u16 reg)
{
	if (cpu_class_is_omap1() || cpu_is_omap2420())
		return __raw_readw(io_base + reg);
	else
		return __raw_readl(io_base + reg);
}

#define OMAP_MCBSP_READ(base, reg) \
			omap_mcbsp_read(base, OMAP_MCBSP_REG_##reg)
#define OMAP_MCBSP_WRITE(base, reg, val) \
			omap_mcbsp_write(base, OMAP_MCBSP_REG_##reg, val)

#define omap_mcbsp_check_valid_id(id)	(id < omap_mcbsp_count)
#define id_to_mcbsp_ptr(id)		mcbsp_ptr[id];

static void omap_mcbsp_dump_reg(u8 id)
{
	struct omap_mcbsp *mcbsp = id_to_mcbsp_ptr(id);

	printk(KERN_EMERG   "This is a KERN_EMERG message. See http://www.xml.com/ldd/chapter/book/ch04.html for descriptions.\n");
	printk(KERN_ALERT   "This is a KERN_ALERT message. \n");
	printk(KERN_CRIT    "This is a KERN_CRIT message. \n");
	printk(KERN_ERR     "This is a KERN_ERR message. \n");
	printk(KERN_WARNING "This is a KERN_WARNING message. \n");
	printk(KERN_NOTICE  "This is a KERN_NOTICE message. \n");
	printk(KERN_INFO    "This is a KERN_INFO message. \n");
	printk(KERN_DEBUG   "This is a KERN_DEBUG message. \n");

	dev_info(mcbsp->dev, "**** McBSP%d regs ****\n", mcbsp->id);
	dev_info(mcbsp->dev, "DRR2:  0x%04x\n",
			OMAP_MCBSP_READ(mcbsp->io_base, DRR2));
	dev_info(mcbsp->dev, "DRR:  0x%04x\n",
			OMAP_MCBSP_READ(mcbsp->io_base, DRR));
	dev_info(mcbsp->dev, "DXR2:  0x%04x\n",
			OMAP_MCBSP_READ(mcbsp->io_base, DXR2));
	dev_info(mcbsp->dev, "DXR:  0x%04x\n",
			OMAP_MCBSP_READ(mcbsp->io_base, DXR));
	dev_info(mcbsp->dev, "SPCR2: 0x%04x\n",
			OMAP_MCBSP_READ(mcbsp->io_base, SPCR2));
	dev_info(mcbsp->dev, "SPCR1: 0x%04x\n",
			OMAP_MCBSP_READ(mcbsp->io_base, SPCR1));
	dev_info(mcbsp->dev, "RCR2:  0x%04x\n",
			OMAP_MCBSP_READ(mcbsp->io_base, RCR2));
	dev_info(mcbsp->dev, "RCR1:  0x%04x\n",
			OMAP_MCBSP_READ(mcbsp->io_base, RCR1));
	dev_info(mcbsp->dev, "XCR2:  0x%04x\n",
			OMAP_MCBSP_READ(mcbsp->io_base, XCR2));
	dev_info(mcbsp->dev, "XCR1:  0x%04x\n",
			OMAP_MCBSP_READ(mcbsp->io_base, XCR1));
	dev_info(mcbsp->dev, "SRGR2: 0x%04x\n",
			OMAP_MCBSP_READ(mcbsp->io_base, SRGR2));
	dev_info(mcbsp->dev, "SRGR1: 0x%04x\n",
			OMAP_MCBSP_READ(mcbsp->io_base, SRGR1));
	dev_info(mcbsp->dev, "PCR0:  0x%04x\n",
			OMAP_MCBSP_READ(mcbsp->io_base, PCR0));
	dev_info(mcbsp->dev, "***********************\n");
	dev_info(mcbsp->dev, "SYSCON:  0x%04x\n",
			OMAP_MCBSP_READ(mcbsp->io_base, SYSCON));
	dev_info(mcbsp->dev, "THRSH1:  0x%04x\n",
			OMAP_MCBSP_READ(mcbsp->io_base, THRSH1));
	dev_info(mcbsp->dev, "THRSH2:  0x%04x\n",
			OMAP_MCBSP_READ(mcbsp->io_base, THRSH2));
	dev_info(mcbsp->dev, "IRQST:  0x%04x\n",
			OMAP_MCBSP_READ(mcbsp->io_base, IRQST));
	dev_info(mcbsp->dev, "IRQEN:  0x%04x\n",
			OMAP_MCBSP_READ(mcbsp->io_base, IRQEN));
	dev_info(mcbsp->dev, "WAKEUPEN:  0x%04x\n",
			OMAP_MCBSP_READ(mcbsp->io_base, WAKEUPEN));
	dev_info(mcbsp->dev, "XCCR:  0x%04x\n",
			OMAP_MCBSP_READ(mcbsp->io_base, XCCR));
	dev_info(mcbsp->dev, "RCCR:  0x%04x\n",
			OMAP_MCBSP_READ(mcbsp->io_base, RCCR));
	dev_info(mcbsp->dev, "***********************\n");
}

static irqreturn_t omap_mcbsp_tx_irq_handler(int irq, void *dev_id)
{
	struct omap_mcbsp *mcbsp_tx = dev_id;
	u16 irqst_spcr2;

	irqst_spcr2 = OMAP_MCBSP_READ(mcbsp_tx->io_base, SPCR2);
	dev_info(mcbsp_tx->dev, "TX IRQ callback : 0x%x\n", irqst_spcr2);

	if (irqst_spcr2 & XSYNC_ERR) {
		dev_err(mcbsp_tx->dev, "TX Frame Sync Error! : 0x%x\n",
			irqst_spcr2);
		/* Writing zero to XSYNC_ERR clears the IRQ */
		OMAP_MCBSP_WRITE(mcbsp_tx->io_base, SPCR2,
			irqst_spcr2 & ~(XSYNC_ERR));
	} else {
		complete(&mcbsp_tx->tx_irq_completion);
	}

	return IRQ_HANDLED;
}

static irqreturn_t omap_mcbsp_rx_irq_handler(int irq, void *dev_id)
{
	struct omap_mcbsp *mcbsp_rx = dev_id;
	u16 irqst_spcr1;

	irqst_spcr1 = OMAP_MCBSP_READ(mcbsp_rx->io_base, SPCR1);
	dev_info(mcbsp_rx->dev, "RX IRQ callback : 0x%x\n", irqst_spcr1);

	if (irqst_spcr1 & RSYNC_ERR) {
		dev_err(mcbsp_rx->dev, "RX Frame Sync Error! : 0x%x\n",
			irqst_spcr1);
		/* Writing zero to RSYNC_ERR clears the IRQ */
		OMAP_MCBSP_WRITE(mcbsp_rx->io_base, SPCR1,
			irqst_spcr1 & ~(RSYNC_ERR));
	} else {
		complete(&mcbsp_rx->tx_irq_completion);
	}

	return IRQ_HANDLED;
}

static void omap_mcbsp_tx_dma_callback(int lch, u16 ch_status, void *data)
{
	struct omap_mcbsp *mcbsp_dma_tx = data;

	dev_info(mcbsp_dma_tx->dev, "TX DMA callback : 0x%x\n",
		OMAP_MCBSP_READ(mcbsp_dma_tx->io_base, SPCR2));

	/* We can free the channels */
	omap_free_dma(mcbsp_dma_tx->dma_tx_lch);
	mcbsp_dma_tx->dma_tx_lch = -1;

	complete(&mcbsp_dma_tx->tx_dma_completion);
}

static void omap_mcbsp_rx_dma_callback(int lch, u16 ch_status, void *data)
{
	struct omap_mcbsp *mcbsp_dma_rx = data;

	dev_info(mcbsp_dma_rx->dev, "RX DMA callback : 0x%x\n",
		OMAP_MCBSP_READ(mcbsp_dma_rx->io_base, SPCR2));

	/* We can free the channels */
	omap_free_dma(mcbsp_dma_rx->dma_rx_lch);
	mcbsp_dma_rx->dma_rx_lch = -1;

	complete(&mcbsp_dma_rx->rx_dma_completion);
}

/*
 * omap_mcbsp_config simply write a config to the
 * appropriate McBSP.
 * You either call this function or set the McBSP registers
 * by yourself before calling omap_mcbsp_start().
 */
void omap_mcbsp_config(unsigned int id, const struct omap_mcbsp_reg_cfg *config)
{
	struct omap_mcbsp *mcbsp;
	void __iomem *io_base;

	if (!omap_mcbsp_check_valid_id(id)) {
		printk(KERN_ERR "%s: Invalid id (%d)\n", __func__, id + 1);
		return;
	}
	mcbsp = id_to_mcbsp_ptr(id);

	io_base = mcbsp->io_base;
	dev_info(mcbsp->dev, "Configuring McBSP%d  phys_base: 0x%08lx\n",
			mcbsp->id, mcbsp->phys_base);

	/* We write the given config */
	OMAP_MCBSP_WRITE(io_base, SPCR2, config->spcr2);
	OMAP_MCBSP_WRITE(io_base, SPCR1, config->spcr1);
	OMAP_MCBSP_WRITE(io_base, RCR2, config->rcr2);
	OMAP_MCBSP_WRITE(io_base, RCR1, config->rcr1);
	OMAP_MCBSP_WRITE(io_base, XCR2, config->xcr2);
	OMAP_MCBSP_WRITE(io_base, XCR1, config->xcr1);
	OMAP_MCBSP_WRITE(io_base, SRGR2, config->srgr2);
	OMAP_MCBSP_WRITE(io_base, SRGR1, config->srgr1);
	OMAP_MCBSP_WRITE(io_base, MCR2, config->mcr2);
	OMAP_MCBSP_WRITE(io_base, MCR1, config->mcr1);
	OMAP_MCBSP_WRITE(io_base, PCR0, config->pcr0);
	if (cpu_is_omap2430() || cpu_is_omap34xx()) {
		OMAP_MCBSP_WRITE(io_base, XCCR, config->xccr);
		OMAP_MCBSP_WRITE(io_base, RCCR, config->rccr);
	}
}
EXPORT_SYMBOL(omap_mcbsp_config);

#ifdef CONFIG_ARCH_OMAP34XX
/*
 * omap_mcbsp_set_tx_threshold configures how to deal
 * with transmit threshold. the threshold value and handler can be
 * configure in here.
 */
void omap_mcbsp_set_tx_threshold(unsigned int id, u16 threshold)
{
	struct omap_mcbsp *mcbsp;
	void __iomem *io_base;

	if (!cpu_is_omap34xx())
		return;

	if (!omap_mcbsp_check_valid_id(id)) {
		printk(KERN_ERR "%s: Invalid id (%d)\n", __func__, id + 1);
		return;
	}
	mcbsp = id_to_mcbsp_ptr(id);
	io_base = mcbsp->io_base;

	OMAP_MCBSP_WRITE(io_base, THRSH2, threshold);
}
EXPORT_SYMBOL(omap_mcbsp_set_tx_threshold);

/*
 * omap_mcbsp_set_rx_threshold configures how to deal
 * with receive threshold. the threshold value and handler can be
 * configure in here.
 */
void omap_mcbsp_set_rx_threshold(unsigned int id, u16 threshold)
{
	struct omap_mcbsp *mcbsp;
	void __iomem *io_base;

	if (!cpu_is_omap34xx())
		return;

	if (!omap_mcbsp_check_valid_id(id)) {
		printk(KERN_ERR "%s: Invalid id (%d)\n", __func__, id + 1);
		return;
	}
	mcbsp = id_to_mcbsp_ptr(id);
	io_base = mcbsp->io_base;

	OMAP_MCBSP_WRITE(io_base, THRSH1, threshold);
}
EXPORT_SYMBOL(omap_mcbsp_set_rx_threshold);

/*
 * omap_mcbsp_get_max_tx_thres just return the current configured
 * maximum threshold for transmission
 */
u16 omap_mcbsp_get_max_tx_threshold(unsigned int id)
{
	struct omap_mcbsp *mcbsp;

	if (!omap_mcbsp_check_valid_id(id)) {
		printk(KERN_ERR "%s: Invalid id (%d)\n", __func__, id + 1);
		return -ENODEV;
	}
	mcbsp = id_to_mcbsp_ptr(id);

	return mcbsp->max_tx_thres;
}
EXPORT_SYMBOL(omap_mcbsp_get_max_tx_threshold);

/*
 * omap_mcbsp_get_max_rx_thres just return the current configured
 * maximum threshold for reception
 */
u16 omap_mcbsp_get_max_rx_threshold(unsigned int id)
{
	struct omap_mcbsp *mcbsp;

	if (!omap_mcbsp_check_valid_id(id)) {
		printk(KERN_ERR "%s: Invalid id (%d)\n", __func__, id + 1);
		return -ENODEV;
	}
	mcbsp = id_to_mcbsp_ptr(id);

	return mcbsp->max_rx_thres;
}
EXPORT_SYMBOL(omap_mcbsp_get_max_rx_threshold);

/*
 * omap_mcbsp_get_dma_op_mode just return the current configured
 * operating mode for the mcbsp channel
 */
int omap_mcbsp_get_dma_op_mode(unsigned int id)
{
	struct omap_mcbsp *mcbsp;
	int dma_op_mode;

	if (!omap_mcbsp_check_valid_id(id)) {
		printk(KERN_ERR "%s: Invalid id (%u)\n", __func__, id + 1);
		return -ENODEV;
	}
	mcbsp = id_to_mcbsp_ptr(id);

	dma_op_mode = mcbsp->dma_op_mode;

	return dma_op_mode;
}
EXPORT_SYMBOL(omap_mcbsp_get_dma_op_mode);

static inline void omap34xx_mcbsp_request(struct omap_mcbsp *mcbsp)
{
	/*
	 * Enable wakup behavior, smart idle and all wakeups
	 * REVISIT: some wakeups may be unnecessary
	 */
	if (cpu_is_omap34xx()) {
		u16 syscon;

		syscon = OMAP_MCBSP_READ(mcbsp->io_base, SYSCON);
		syscon &= ~(ENAWAKEUP | SIDLEMODE(0x03) | CLOCKACTIVITY(0x03));

		if (mcbsp->dma_op_mode == MCBSP_DMA_MODE_THRESHOLD) {
			syscon |= (ENAWAKEUP | SIDLEMODE(0x02) |
					CLOCKACTIVITY(0x02));
			OMAP_MCBSP_WRITE(mcbsp->io_base, WAKEUPEN,
					XRDYEN | RRDYEN);
		} else {
			syscon |= SIDLEMODE(0x01);
		}

		OMAP_MCBSP_WRITE(mcbsp->io_base, SYSCON, syscon);
	}
}

static inline void omap34xx_mcbsp_free(struct omap_mcbsp *mcbsp)
{
	/*
	 * Disable wakup behavior, smart idle and all wakeups
	 */
	if (cpu_is_omap34xx()) {
		u16 syscon;

		syscon = OMAP_MCBSP_READ(mcbsp->io_base, SYSCON);
		syscon &= ~(ENAWAKEUP | SIDLEMODE(0x03) | CLOCKACTIVITY(0x03));
		/*
		 * HW bug workaround - If no_idle mode is taken, we need to
		 * go to smart_idle before going to always_idle, or the
		 * device will not hit retention anymore.
		 */
		syscon |= SIDLEMODE(0x02);
		OMAP_MCBSP_WRITE(mcbsp->io_base, SYSCON, syscon);

		syscon &= ~(SIDLEMODE(0x03));
		OMAP_MCBSP_WRITE(mcbsp->io_base, SYSCON, syscon);

		OMAP_MCBSP_WRITE(mcbsp->io_base, WAKEUPEN, 0);
	}
}
#else
static inline void omap34xx_mcbsp_request(struct omap_mcbsp *mcbsp) {}
static inline void omap34xx_mcbsp_free(struct omap_mcbsp *mcbsp) {}
#endif

/*
 * We can choose between IRQ based or polled IO.
 * This needs to be called before omap_mcbsp_request().
 */
int omap_mcbsp_set_io_type(unsigned int id, omap_mcbsp_io_type_t io_type)
{
	struct omap_mcbsp *mcbsp;

	if (!omap_mcbsp_check_valid_id(id)) {
		printk(KERN_ERR "%s: Invalid id (%d)\n", __func__, id + 1);
		return -ENODEV;
	}
	mcbsp = id_to_mcbsp_ptr(id);

	spin_lock(&mcbsp->lock);

	if (!mcbsp->free) {
		dev_err(mcbsp->dev, "McBSP%d is currently in use\n",
			mcbsp->id);
		spin_unlock(&mcbsp->lock);
		return -EINVAL;
	}

	mcbsp->io_type = io_type;

	spin_unlock(&mcbsp->lock);

	return 0;
}
EXPORT_SYMBOL(omap_mcbsp_set_io_type);

int omap_mcbsp_request(unsigned int id)
{
	struct omap_mcbsp *mcbsp;
	int err;

	if (!omap_mcbsp_check_valid_id(id)) {
		printk(KERN_ERR "%s: Invalid id (%d)\n", __func__, id + 1);
		return -ENODEV;
	}
	mcbsp = id_to_mcbsp_ptr(id);

	spin_lock(&mcbsp->lock);
	if (!mcbsp->free) {
		dev_err(mcbsp->dev, "McBSP%d is currently in use\n",
			mcbsp->id);
		spin_unlock(&mcbsp->lock);
		return -EBUSY;
	}

	mcbsp->free = 0;
	spin_unlock(&mcbsp->lock);

	if (mcbsp->pdata && mcbsp->pdata->ops && mcbsp->pdata->ops->request)
		mcbsp->pdata->ops->request(id);

	clk_enable(mcbsp->iclk);
	clk_enable(mcbsp->fclk);

	/*
	 * Make sure that transmitter, receiver and sample-rate generator are
	 * not running before activating IRQs.
	 */
	OMAP_MCBSP_WRITE(mcbsp->io_base, SPCR1, 0);
	OMAP_MCBSP_WRITE(mcbsp->io_base, SPCR2, 0);

	if (mcbsp->io_type == OMAP_MCBSP_IRQ_IO) {
		/* We need to get IRQs here */
		init_completion(&mcbsp->tx_irq_completion);
		err = request_irq(mcbsp->tx_irq, omap_mcbsp_tx_irq_handler,
					0, "McBSP", (void *)mcbsp);
		if (err != 0) {
			dev_err(mcbsp->dev, "Unable to request TX IRQ %d "
					"for McBSP%d\n", mcbsp->tx_irq,
					mcbsp->id);
			return err;
		}

		init_completion(&mcbsp->rx_irq_completion);
		err = request_irq(mcbsp->rx_irq, omap_mcbsp_rx_irq_handler,
					0, "McBSP", (void *)mcbsp);
		if (err != 0) {
			dev_err(mcbsp->dev, "Unable to request RX IRQ %d "
					"for McBSP%d\n", mcbsp->rx_irq,
					mcbsp->id);
			free_irq(mcbsp->tx_irq, (void *)mcbsp);
			return err;
		}
	}

	return 0;
}
EXPORT_SYMBOL(omap_mcbsp_request);

void omap_mcbsp_free(unsigned int id)
{
	struct omap_mcbsp *mcbsp;

	if (!omap_mcbsp_check_valid_id(id)) {
		printk(KERN_ERR "%s: Invalid id (%d)\n", __func__, id + 1);
		return;
	}
	mcbsp = id_to_mcbsp_ptr(id);

	if (mcbsp->pdata && mcbsp->pdata->ops && mcbsp->pdata->ops->free)
		mcbsp->pdata->ops->free(id);

	clk_disable(mcbsp->fclk);
	clk_disable(mcbsp->iclk);

	if (mcbsp->io_type == OMAP_MCBSP_IRQ_IO) {
		/* Free IRQs */
		free_irq(mcbsp->rx_irq, (void *)mcbsp);
		free_irq(mcbsp->tx_irq, (void *)mcbsp);
	}

	spin_lock(&mcbsp->lock);
	if (mcbsp->free) {
		dev_err(mcbsp->dev, "McBSP%d was not reserved\n",
			mcbsp->id);
		spin_unlock(&mcbsp->lock);
		return;
	}

	mcbsp->free = 1;
	spin_unlock(&mcbsp->lock);
}
EXPORT_SYMBOL(omap_mcbsp_free);

/*
 * Here we start the McBSP, by enabling the sample
 * generator, both transmitter and receivers,
 * and the frame sync.
 */
void omap_mcbsp_start(unsigned int id)
{
	struct omap_mcbsp *mcbsp;
	void __iomem *io_base;
	u16 w;

	if (!omap_mcbsp_check_valid_id(id)) {
		printk(KERN_ERR "%s: Invalid id (%d)\n", __func__, id + 1);
		return;
	}
	mcbsp = id_to_mcbsp_ptr(id);
	io_base = mcbsp->io_base;

	mcbsp->rx_word_length = (OMAP_MCBSP_READ(io_base, RCR1) >> 5) & 0x7;
	mcbsp->tx_word_length = (OMAP_MCBSP_READ(io_base, XCR1) >> 5) & 0x7;

	/* Start the sample generator */
	w = OMAP_MCBSP_READ(io_base, SPCR2);
	OMAP_MCBSP_WRITE(io_base, SPCR2, w | (1 << 6));

	/* Enable transmitter and receiver */
	w = OMAP_MCBSP_READ(io_base, SPCR2);
	OMAP_MCBSP_WRITE(io_base, SPCR2, w | 1);

	w = OMAP_MCBSP_READ(io_base, SPCR1);
	OMAP_MCBSP_WRITE(io_base, SPCR1, w | 1);

	udelay(100);

	/* Start frame sync */
	w = OMAP_MCBSP_READ(io_base, SPCR2);
	OMAP_MCBSP_WRITE(io_base, SPCR2, w | (1 << 7));

	/* Dump McBSP Regs */
	omap_mcbsp_dump_reg(id);
}
EXPORT_SYMBOL(omap_mcbsp_start);

void omap_mcbsp_stop(unsigned int id)
{
	struct omap_mcbsp *mcbsp;
	void __iomem *io_base;
	u16 w;

	if (!omap_mcbsp_check_valid_id(id)) {
		printk(KERN_ERR "%s: Invalid id (%d)\n", __func__, id + 1);
		return;
	}

	mcbsp = id_to_mcbsp_ptr(id);
	io_base = mcbsp->io_base;

	/* Reset transmitter */
	w = OMAP_MCBSP_READ(io_base, SPCR2);
	OMAP_MCBSP_WRITE(io_base, SPCR2, w & ~(1));

	/* Reset receiver */
	w = OMAP_MCBSP_READ(io_base, SPCR1);
	OMAP_MCBSP_WRITE(io_base, SPCR1, w & ~(1));

	/* Reset the sample rate generator */
	w = OMAP_MCBSP_READ(io_base, SPCR2);
	OMAP_MCBSP_WRITE(io_base, SPCR2, w & ~(1 << 6));
}
EXPORT_SYMBOL(omap_mcbsp_stop);

/* polled mcbsp i/o operations */
int omap_mcbsp_pollwrite(unsigned int id, u16 buf)
{
	struct omap_mcbsp *mcbsp;
	void __iomem *base;

	if (!omap_mcbsp_check_valid_id(id)) {
		printk(KERN_ERR "%s: Invalid id (%d)\n", __func__, id + 1);
		return -ENODEV;
	}

	mcbsp = id_to_mcbsp_ptr(id);
	base = mcbsp->io_base;

	writew(buf, base + OMAP_MCBSP_REG_DXR);
	/* if frame sync error - clear the error */
	if (readw(base + OMAP_MCBSP_REG_SPCR2) & XSYNC_ERR) {
		/* clear error */
		writew(readw(base + OMAP_MCBSP_REG_SPCR2) & (~XSYNC_ERR),
		       base + OMAP_MCBSP_REG_SPCR2);
		/* resend */
		return -1;
	} else {
		/* wait for transmit confirmation */
		int attemps = 0;
		while (!(readw(base + OMAP_MCBSP_REG_SPCR2) & XRDY)) {
			if (attemps++ > 1000) {
				writew(readw(base + OMAP_MCBSP_REG_SPCR2) &
				       (~XRST),
				       base + OMAP_MCBSP_REG_SPCR2);
				udelay(10);
				writew(readw(base + OMAP_MCBSP_REG_SPCR2) |
				       (XRST),
				       base + OMAP_MCBSP_REG_SPCR2);
				udelay(10);
				dev_err(mcbsp->dev, "Could not write to"
					" McBSP%d Register\n", mcbsp->id);
				return -2;
			}
		}
	}

	return 0;
}
EXPORT_SYMBOL(omap_mcbsp_pollwrite);

int omap_mcbsp_pollread(unsigned int id, u16 *buf)
{
	struct omap_mcbsp *mcbsp;
	void __iomem *base;

	if (!omap_mcbsp_check_valid_id(id)) {
		printk(KERN_ERR "%s: Invalid id (%d)\n", __func__, id + 1);
		return -ENODEV;
	}
	mcbsp = id_to_mcbsp_ptr(id);

	base = mcbsp->io_base;
	/* if frame sync error - clear the error */
	if (readw(base + OMAP_MCBSP_REG_SPCR1) & RSYNC_ERR) {
		/* clear error */
		writew(readw(base + OMAP_MCBSP_REG_SPCR1) & (~RSYNC_ERR),
		       base + OMAP_MCBSP_REG_SPCR1);
		/* resend */
		return -1;
	} else {
		/* wait for recieve confirmation */
		int attemps = 0;
		while (!(readw(base + OMAP_MCBSP_REG_SPCR1) & RRDY)) {
			if (attemps++ > 1000) {
				writew(readw(base + OMAP_MCBSP_REG_SPCR1) &
				       (~RRST),
				       base + OMAP_MCBSP_REG_SPCR1);
				udelay(10);
				writew(readw(base + OMAP_MCBSP_REG_SPCR1) |
				       (RRST),
				       base + OMAP_MCBSP_REG_SPCR1);
				udelay(10);
				dev_err(mcbsp->dev, "Could not read from"
					" McBSP%d Register\n", mcbsp->id);
				return -2;
			}
		}
	}
	*buf = readw(base + OMAP_MCBSP_REG_DRR);

	return 0;
}
EXPORT_SYMBOL(omap_mcbsp_pollread);

/*
 * IRQ based word transmission.
 */
void omap_mcbsp_xmit_word(unsigned int id, u32 word)
{
	struct omap_mcbsp *mcbsp;
	void __iomem *io_base;
	omap_mcbsp_word_length word_length;

	if (!omap_mcbsp_check_valid_id(id)) {
		printk(KERN_ERR "%s: Invalid id (%d)\n", __func__, id + 1);
		return;
	}

	mcbsp = id_to_mcbsp_ptr(id);
	io_base = mcbsp->io_base;
	word_length = mcbsp->tx_word_length;

	wait_for_completion(&mcbsp->tx_irq_completion);

//	if (word_length > OMAP_MCBSP_WORD_16)
//		OMAP_MCBSP_WRITE(io_base, DXR2, word >> 16);
	OMAP_MCBSP_WRITE(io_base, DXR, word & 0xffff);
}
EXPORT_SYMBOL(omap_mcbsp_xmit_word);

u32 omap_mcbsp_recv_word(unsigned int id)
{
	struct omap_mcbsp *mcbsp;
	void __iomem *io_base;
	u16 word_lsb, word_msb = 0;
	omap_mcbsp_word_length word_length;

	if (!omap_mcbsp_check_valid_id(id)) {
		printk(KERN_ERR "%s: Invalid id (%d)\n", __func__, id + 1);
		return -ENODEV;
	}
	mcbsp = id_to_mcbsp_ptr(id);

	word_length = mcbsp->rx_word_length;
	io_base = mcbsp->io_base;

	wait_for_completion(&mcbsp->rx_irq_completion);

	if (word_length > OMAP_MCBSP_WORD_16)
		word_msb = OMAP_MCBSP_READ(io_base, DRR2);
	word_lsb = OMAP_MCBSP_READ(io_base, DRR);

	return (word_lsb | (word_msb << 16));
}
EXPORT_SYMBOL(omap_mcbsp_recv_word);

int omap_mcbsp_spi_master_xmit_word_poll(unsigned int id, u32 word)
{
	struct omap_mcbsp *mcbsp;
	void __iomem *io_base;
	omap_mcbsp_word_length tx_word_length;
	omap_mcbsp_word_length rx_word_length;
	u16 spcr2, spcr1, attempts = 0, word_lsb, word_msb = 0;

	if (!omap_mcbsp_check_valid_id(id)) {
		printk(KERN_ERR "%s: Invalid id (%d)\n", __func__, id + 1);
		return -ENODEV;
	}
	mcbsp = id_to_mcbsp_ptr(id);
	io_base = mcbsp->io_base;
	tx_word_length = mcbsp->tx_word_length;
	rx_word_length = mcbsp->rx_word_length;

	if (tx_word_length != rx_word_length)
		return -EINVAL;

	/* First we wait for the transmitter to be ready */
	spcr2 = OMAP_MCBSP_READ(io_base, SPCR2);
	while (!(spcr2 & XRDY)) {
		spcr2 = OMAP_MCBSP_READ(io_base, SPCR2);
		if (attempts++ > 1000) {
			/* We must reset the transmitter */
			OMAP_MCBSP_WRITE(io_base, SPCR2, spcr2 & (~XRST));
			udelay(10);
			OMAP_MCBSP_WRITE(io_base, SPCR2, spcr2 | XRST);
			udelay(10);
			dev_err(mcbsp->dev, "McBSP%d transmitter not "
				"ready\n", mcbsp->id);
			return -EAGAIN;
		}
	}

	/* Now we can push the data */
//	if (tx_word_length > OMAP_MCBSP_WORD_16)
//		OMAP_MCBSP_WRITE(io_base, DXR2, word >> 16);
	OMAP_MCBSP_WRITE(io_base, DXR, word & 0xffff);

	/* We wait for the receiver to be ready */
	spcr1 = OMAP_MCBSP_READ(io_base, SPCR1);
	while (!(spcr1 & RRDY)) {
		spcr1 = OMAP_MCBSP_READ(io_base, SPCR1);
		if (attempts++ > 1000) {
			/* We must reset the receiver */
			OMAP_MCBSP_WRITE(io_base, SPCR1, spcr1 & (~RRST));
			udelay(10);
			OMAP_MCBSP_WRITE(io_base, SPCR1, spcr1 | RRST);
			udelay(10);
			dev_err(mcbsp->dev, "McBSP%d receiver not "
				"ready\n", mcbsp->id);
			return -EAGAIN;
		}
	}

	/* Receiver is ready, let's read the dummy data */
//	if (rx_word_length > OMAP_MCBSP_WORD_16)
//		word_msb = OMAP_MCBSP_READ(io_base, DRR2);
	word_lsb = OMAP_MCBSP_READ(io_base, DRR);

	return 0;
}
EXPORT_SYMBOL(omap_mcbsp_spi_master_xmit_word_poll);

int omap_mcbsp_spi_master_recv_word_poll(unsigned int id, u32 *word)
{
	struct omap_mcbsp *mcbsp;
	u32 clock_word = 0;
	void __iomem *io_base;
	omap_mcbsp_word_length tx_word_length;
	omap_mcbsp_word_length rx_word_length;
	u16 spcr2, spcr1, attempts = 0, word_lsb, word_msb = 0;

	if (!omap_mcbsp_check_valid_id(id)) {
		printk(KERN_ERR "%s: Invalid id (%d)\n", __func__, id + 1);
		return -ENODEV;
	}

	mcbsp = id_to_mcbsp_ptr(id);
	io_base = mcbsp->io_base;

	tx_word_length = mcbsp->tx_word_length;
	rx_word_length = mcbsp->rx_word_length;

	if (tx_word_length != rx_word_length)
		return -EINVAL;

	/* First we wait for the transmitter to be ready */
	spcr2 = OMAP_MCBSP_READ(io_base, SPCR2);
	while (!(spcr2 & XRDY)) {
		spcr2 = OMAP_MCBSP_READ(io_base, SPCR2);
		if (attempts++ > 1000) {
			/* We must reset the transmitter */
			OMAP_MCBSP_WRITE(io_base, SPCR2, spcr2 & (~XRST));
			udelay(10);
			OMAP_MCBSP_WRITE(io_base, SPCR2, spcr2 | XRST);
			udelay(10);
			dev_err(mcbsp->dev, "McBSP%d transmitter not "
				"ready\n", mcbsp->id);
			return -EAGAIN;
		}
	}

	/* We first need to enable the bus clock */
//	if (tx_word_length > OMAP_MCBSP_WORD_16)
//		OMAP_MCBSP_WRITE(io_base, DXR2, clock_word >> 16);
	OMAP_MCBSP_WRITE(io_base, DXR, clock_word & 0xffff);

	/* We wait for the receiver to be ready */
	spcr1 = OMAP_MCBSP_READ(io_base, SPCR1);
	while (!(spcr1 & RRDY)) {
		spcr1 = OMAP_MCBSP_READ(io_base, SPCR1);
		if (attempts++ > 1000) {
			/* We must reset the receiver */
			OMAP_MCBSP_WRITE(io_base, SPCR1, spcr1 & (~RRST));
			udelay(10);
			OMAP_MCBSP_WRITE(io_base, SPCR1, spcr1 | RRST);
			udelay(10);
			dev_err(mcbsp->dev, "McBSP%d receiver not "
				"ready\n", mcbsp->id);
			return -EAGAIN;
		}
	}

	/* Receiver is ready, there is something for us */
//	if (rx_word_length > OMAP_MCBSP_WORD_16)
//		word_msb = OMAP_MCBSP_READ(io_base, DRR2);
	word_lsb = OMAP_MCBSP_READ(io_base, DRR);

	word[0] = (word_lsb | (word_msb << 16));

	return 0;
}
EXPORT_SYMBOL(omap_mcbsp_spi_master_recv_word_poll);

/*
 * Simple DMA based buffer rx/tx routines.
 * Nothing fancy, just a single buffer tx/rx through DMA.
 * The DMA resources are released once the transfer is done.
 * For anything fancier, you should use your own customized DMA
 * routines and callbacks.
 */
int omap_mcbsp_xmit_buffer(unsigned int id, dma_addr_t buffer,
				unsigned int length)
{
	struct omap_mcbsp *mcbsp;
	int dma_tx_ch;
	int src_port = 0;
	int dest_port = 0;
	int sync_dev = 0;

	if (!omap_mcbsp_check_valid_id(id)) {
		printk(KERN_ERR "%s: Invalid id (%d)\n", __func__, id + 1);
		return -ENODEV;
	}
	mcbsp = id_to_mcbsp_ptr(id);

	if (omap_request_dma(mcbsp->dma_tx_sync, "McBSP TX",
				omap_mcbsp_tx_dma_callback,
				mcbsp,
				&dma_tx_ch)) {
		dev_err(mcbsp->dev, " Unable to request DMA channel for "
				"McBSP%d TX. Trying IRQ based TX\n",
				mcbsp->id);
		return -EAGAIN;
	}
	mcbsp->dma_tx_lch = dma_tx_ch;

	dev_err(mcbsp->dev, "McBSP%d TX DMA on channel %d\n", mcbsp->id,
		dma_tx_ch);

	init_completion(&mcbsp->tx_dma_completion);

	if (cpu_class_is_omap1()) {
		src_port = OMAP_DMA_PORT_TIPB;
		dest_port = OMAP_DMA_PORT_EMIFF;
	}
	if (cpu_class_is_omap2())
		sync_dev = mcbsp->dma_tx_sync;

	omap_set_dma_transfer_params(mcbsp->dma_tx_lch,
				     OMAP_DMA_DATA_TYPE_S32,
				     length >> 1, 1,
				     OMAP_DMA_SYNC_ELEMENT,
	 sync_dev, 0);

	omap_set_dma_dest_params(mcbsp->dma_tx_lch,
				 src_port,
				 OMAP_DMA_AMODE_CONSTANT,
				 mcbsp->phys_base + OMAP_MCBSP_REG_DXR,
				 0, 0);

	omap_set_dma_src_params(mcbsp->dma_tx_lch,
				dest_port,
				OMAP_DMA_AMODE_POST_INC,
				buffer,
				0, 0);

	omap_start_dma(mcbsp->dma_tx_lch);
	wait_for_completion(&mcbsp->tx_dma_completion);

	return 0;
}
EXPORT_SYMBOL(omap_mcbsp_xmit_buffer);

int omap_mcbsp_recv_buffer(unsigned int id, dma_addr_t buffer,
				unsigned int length)
{
	struct omap_mcbsp *mcbsp;
	int dma_rx_ch;
	int src_port = 0;
	int dest_port = 0;
	int sync_dev = 0;

	if (!omap_mcbsp_check_valid_id(id)) {
		printk(KERN_ERR "%s: Invalid id (%d)\n", __func__, id + 1);
		return -ENODEV;
	}
	mcbsp = id_to_mcbsp_ptr(id);

	if (omap_request_dma(mcbsp->dma_rx_sync, "McBSP RX",
				omap_mcbsp_rx_dma_callback,
				mcbsp,
				&dma_rx_ch)) {
		dev_err(mcbsp->dev, "Unable to request DMA channel for "
				"McBSP%d RX. Trying IRQ based RX\n",
				mcbsp->id);
		return -EAGAIN;
	}
	mcbsp->dma_rx_lch = dma_rx_ch;

	dev_err(mcbsp->dev, "McBSP%d RX DMA on channel %d\n", mcbsp->id,
		dma_rx_ch);

	init_completion(&mcbsp->rx_dma_completion);

	if (cpu_class_is_omap1()) {
		src_port = OMAP_DMA_PORT_TIPB;
		dest_port = OMAP_DMA_PORT_EMIFF;
	}
	if (cpu_class_is_omap2())
		sync_dev = mcbsp->dma_rx_sync;

	omap_set_dma_transfer_params(mcbsp->dma_rx_lch,
					OMAP_DMA_DATA_TYPE_S32,
					length >> 1, 1,
					OMAP_DMA_SYNC_FRAME,
					sync_dev, 0);

	omap_set_dma_src_params(mcbsp->dma_rx_lch,
				src_port,
				OMAP_DMA_AMODE_CONSTANT,
				mcbsp->phys_base + OMAP_MCBSP_REG_DRR,
				0, 0);

	omap_set_dma_dest_params(mcbsp->dma_rx_lch,
					dest_port,
					OMAP_DMA_AMODE_POST_INC,
					buffer,
					0, 0);

	omap_start_dma(mcbsp->dma_rx_lch);
	wait_for_completion(&mcbsp->rx_dma_completion);

	return 0;
}
EXPORT_SYMBOL(omap_mcbsp_recv_buffer);

void getMcBSPDevice(unsigned int id, struct omap_mcbsp **mcbsp)
{
	*mcbsp = id_to_mcbsp_ptr(id);
}
EXPORT_SYMBOL(getMcBSPDevice);



/*
 * SPI wrapper.
 * Since SPI setup is much simpler than the generic McBSP one,
 * this wrapper just need an omap_mcbsp_spi_cfg structure as an input.
 * Once this is done, you can call omap_mcbsp_start().
 */
void omap_mcbsp_set_spi_mode(unsigned int id,
				const struct omap_mcbsp_spi_cfg *spi_cfg)
{
	struct omap_mcbsp *mcbsp;
	struct omap_mcbsp_reg_cfg mcbsp_cfg;

	if (!omap_mcbsp_check_valid_id(id)) {
		printk(KERN_ERR "%s: Invalid id (%d)\n", __func__, id + 1);
		return;
	}
	mcbsp = id_to_mcbsp_ptr(id);

	memset(&mcbsp_cfg, 0, sizeof(struct omap_mcbsp_reg_cfg));

	/* SPI has only one frame */
	mcbsp_cfg.rcr1 |= (RWDLEN1(spi_cfg->word_length) | RFRLEN1(0));
	mcbsp_cfg.xcr1 |= (XWDLEN1(spi_cfg->word_length) | XFRLEN1(0));

	/* Clock stop mode */
	if (spi_cfg->clk_stp_mode == OMAP_MCBSP_CLK_STP_MODE_NO_DELAY)
		mcbsp_cfg.spcr1 |= (1 << 12);
	else
		mcbsp_cfg.spcr1 |= (3 << 11);

	/* Set clock parities */
	if (spi_cfg->rx_clock_polarity == OMAP_MCBSP_CLK_RISING)
		mcbsp_cfg.pcr0 |= CLKRP;
	else
		mcbsp_cfg.pcr0 &= ~CLKRP;

	if (spi_cfg->tx_clock_polarity == OMAP_MCBSP_CLK_RISING)
		mcbsp_cfg.pcr0 &= ~CLKXP;
	else
		mcbsp_cfg.pcr0 |= CLKXP;

	/* Set SCLKME to 0 and CLKSM to 1 */
	mcbsp_cfg.pcr0 &= ~SCLKME;
	mcbsp_cfg.srgr2 |= CLKSM;

	/* Set FSXP */
	if (spi_cfg->fsx_polarity == OMAP_MCBSP_FS_ACTIVE_HIGH)
		mcbsp_cfg.pcr0 &= ~FSXP;
	else
		mcbsp_cfg.pcr0 |= FSXP;

	if (spi_cfg->spi_mode == OMAP_MCBSP_SPI_MASTER) {
		mcbsp_cfg.pcr0 |= CLKXM;
		mcbsp_cfg.srgr1 |= CLKGDV(spi_cfg->clk_div - 1);
		mcbsp_cfg.pcr0 |= FSXM;
		mcbsp_cfg.srgr2 &= ~FSGM;
		mcbsp_cfg.xcr2 |= XDATDLY(1);
		mcbsp_cfg.rcr2 |= RDATDLY(1);
	} else {
		mcbsp_cfg.pcr0 &= ~CLKXM;
		mcbsp_cfg.srgr1 |= CLKGDV(1);
		mcbsp_cfg.pcr0 &= ~FSXM;
		mcbsp_cfg.xcr2 &= ~XDATDLY(3);
		mcbsp_cfg.rcr2 &= ~RDATDLY(3);
	}

	mcbsp_cfg.xcr2 &= ~XPHASE;
	mcbsp_cfg.rcr2 &= ~RPHASE;

	omap_mcbsp_config(id, &mcbsp_cfg);
}
EXPORT_SYMBOL(omap_mcbsp_set_spi_mode);

/*
 * McBSP1 and McBSP3 are directly mapped on 1610 and 1510.
 * 730 has only 2 McBSP, and both of them are MPU peripherals.
 */
static int __devinit omap_mcbsp_probe(struct platform_device *pdev)
{
	struct omap_mcbsp_platform_data *pdata = pdev->dev.platform_data;
	struct omap_mcbsp *mcbsp;
	int id = pdev->id - 1;
	int ret = 0;

	if (!pdata) {
		dev_err(&pdev->dev, "McBSP device initialized without"
				"platform data\n");
		ret = -EINVAL;
		goto exit;
	}

	dev_info(&pdev->dev, "Initializing OMAP McBSP (%d).\n", pdev->id);

	if (id >= omap_mcbsp_count) {
		dev_err(&pdev->dev, "Invalid McBSP device id (%d)\n", id);
		ret = -EINVAL;
		goto exit;
	}

	mcbsp = kzalloc(sizeof(struct omap_mcbsp), GFP_KERNEL);
	if (!mcbsp) {
		ret = -ENOMEM;
		goto exit;
	}

	spin_lock_init(&mcbsp->lock);
	mcbsp->id = id + 1;
	mcbsp->free = 1;
	mcbsp->dma_tx_lch = -1;
	mcbsp->dma_rx_lch = -1;

	mcbsp->phys_base = pdata->phys_base;
	mcbsp->io_base = ioremap(pdata->phys_base, SZ_4K);
	if (!mcbsp->io_base) {
		ret = -ENOMEM;
		goto err_ioremap;
	}

	/* Default I/O is IRQ based */
	mcbsp->io_type = OMAP_MCBSP_IRQ_IO;
	mcbsp->tx_irq = pdata->tx_irq;
	mcbsp->rx_irq = pdata->rx_irq;
	mcbsp->dma_rx_sync = pdata->dma_rx_sync;
	mcbsp->dma_tx_sync = pdata->dma_tx_sync;

	mcbsp->iclk = clk_get(&pdev->dev, "ick");
	if (IS_ERR(mcbsp->iclk)) {
		ret = PTR_ERR(mcbsp->iclk);
		dev_err(&pdev->dev, "unable to get ick: %d\n", ret);
		goto err_iclk;
	}

	mcbsp->fclk = clk_get(&pdev->dev, "fck");
	if (IS_ERR(mcbsp->fclk)) {
		ret = PTR_ERR(mcbsp->fclk);
		dev_err(&pdev->dev, "unable to get fck: %d\n", ret);
		goto err_fclk;
	}

	mcbsp->pdata = pdata;
	mcbsp->dev = &pdev->dev;
	mcbsp_ptr[id] = mcbsp;
	platform_set_drvdata(pdev, mcbsp);
	return 0;

err_fclk:
	clk_put(mcbsp->iclk);
err_iclk:
	iounmap(mcbsp->io_base);
err_ioremap:
	kfree(mcbsp);
exit:
	return ret;
}

static int __devexit omap_mcbsp_remove(struct platform_device *pdev)
{
	struct omap_mcbsp *mcbsp = platform_get_drvdata(pdev);

	platform_set_drvdata(pdev, NULL);
	if (mcbsp) {

		if (mcbsp->pdata && mcbsp->pdata->ops &&
				mcbsp->pdata->ops->free)
			mcbsp->pdata->ops->free(mcbsp->id);

		clk_disable(mcbsp->fclk);
		clk_disable(mcbsp->iclk);
		clk_put(mcbsp->fclk);
		clk_put(mcbsp->iclk);

		iounmap(mcbsp->io_base);

		mcbsp->fclk = NULL;
		mcbsp->iclk = NULL;
		mcbsp->free = 0;
		mcbsp->dev = NULL;
	}

	return 0;
}

static struct platform_driver omap_mcbsp_driver = {
	.probe		= omap_mcbsp_probe,
	.remove		= __devexit_p(omap_mcbsp_remove),
	.driver		= {
		.name	= "omap-mcbsp",
	},
};

int __init omap_mcbsp_init(void)
{
	/* Register the McBSP driver */
	return platform_driver_register(&omap_mcbsp_driver);
}
