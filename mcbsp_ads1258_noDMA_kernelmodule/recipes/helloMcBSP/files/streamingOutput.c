#define KERNEL31

#include <config/modversions.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/dma-mapping.h>
#include <linux/gpio.h>

#include <mach/mcbsp.h>
#include <mach/dma.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Simon Vogt <simonsunimail@gmail.com>");

int mcbspID = 0; /*McBSP1 => id 0*/
module_param(mcbspID, int, 0);
MODULE_PARM_DESC(mcbspID, "The McBSP to use. Starts at 0.");

int outval1 = 0x53C3C3CA;
module_param(outval1, int, 0);
MODULE_PARM_DESC(outval1, "The first value to output in a stream.");

int outval2 = 0x5C3C3C3A;
module_param(outval2, int, 0);
MODULE_PARM_DESC(outval2, "The second value to output in a stream.");

int outval3 = 0x5E7E7E7A;
module_param(outval3, int, 0);
MODULE_PARM_DESC(outval3, "The third value to output in a stream.");

int outval4 = 0x5181818A;
module_param(outval4, int, 0);
MODULE_PARM_DESC(outval4, "The fourth value to output in a stream.");

int wordlength = 32;
module_param(wordlength, int, 32);
MODULE_PARM_DESC(wordlength, "The word length in bits that the serial port should send in. Use 8,16 or 32 bits please.");


/* ACTIVE SETTINGS:*/
static struct omap_mcbsp_reg_cfg simon_regs = {
        .spcr2 = XINTM(3),
        .spcr1 = RINTM(3),
        .rcr2  = 0,
        .rcr1  = RFRLEN1(0) | RWDLEN1(OMAP_MCBSP_WORD_32),  // frame is 1 word. word is 32 bits.
        .xcr2  = 0,
        .xcr1  = XFRLEN1(0) | XWDLEN1(OMAP_MCBSP_WORD_32),
        .srgr1 = FWID(31) | CLKGDV(50),
        .srgr2 = /*GSYNC |*/ 0/*CLKSM*/ | CLKSP  | FPER(250),// | FSGM, // see pages 129 to 131 of sprufd1.pdf
        .pcr0  = FSXM | FSRM | CLKXM | CLKRM | FSXP | FSRP | CLKXP | CLKRP,
        //.pcr0 = CLKXP | CLKRP,        /* mcbsp: slave */
	.xccr = DXENDLY(1) | XDMAEN ,//| XDISABLE,
	.rccr = RFULL_CYCLE | RDMAEN,// | RDISABLE,
};

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

static void simon_omap_mcbsp_dump_reg(u8 id)
{
	struct omap_mcbsp *mcbsp;
	getMcBSPDevice(mcbspID,&mcbsp);

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

static void simon_omap_mcbsp_tx_dma_callback(int lch, u16 ch_status, void *data)
{
	struct omap_mcbsp *mcbsp_dma_tx = data;

	dev_info(mcbsp_dma_tx->dev, "TX DMA callback : 0x%x\n",
		OMAP_MCBSP_READ(mcbsp_dma_tx->io_base, SPCR2));

	/* We can free the channels */
	omap_free_dma(mcbsp_dma_tx->dma_tx_lch);
	mcbsp_dma_tx->dma_tx_lch = -1;

	complete(&mcbsp_dma_tx->tx_dma_completion);
}

static void simon_omap_mcbsp_rx_dma_callback(int lch, u16 ch_status, void *data)
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
 * Simple DMA based buffer rx/tx routines.
 * Nothing fancy, just a single buffer tx/rx through DMA.
 * The DMA resources are released once the transfer is done.
 * For anything fancier, you should use your own customized DMA
 * routines and callbacks.
 */
int simon_omap_mcbsp_xmit_buffer(unsigned int id, dma_addr_t buffer,
				unsigned int length)
{
	struct omap_mcbsp *mcbsp;
	int dma_tx_ch;
	int src_port = 0;
	int dest_port = 0;
	int sync_dev = 0;

//	if (!omap_mcbsp_check_valid_id(id)) {
//		printk(KERN_ERR "%s: Invalid id (%d)\n", __func__, id + 1);
//		return -ENODEV;
//	}
//	mcbsp = id_to_mcbsp_ptr(id);
	getMcBSPDevice(mcbspID,&mcbsp);

	if (omap_request_dma(mcbsp->dma_tx_sync, "McBSP TX",
				simon_omap_mcbsp_tx_dma_callback,
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
//EXPORT_SYMBOL(omap_mcbsp_xmit_buffer);


int hello_init(void)
{
	int status = 3;
	int reqstatus = -4;
	u16 value16 = 0;
	u32 value32 = 0;
	int returnstatus = 0;
	u32 mybuffer = 0x5CCC333A; // 01011100110011000011001100111010
	u32 mybuffer2 = 0x53CA; // 0101 00111100 1010
	int i;
	char printtemp[500];

	struct omap_mcbsp *mcbsp;

	int bufbufsize = 128 * 7; // number of array elements
	int bytesPerVal = 4; // number of bytes per array element (32bit = 4 bytes, 16bit = 2 bytes)
	u32* bufbuf1; // the buffer
	u32* bufbuf2; // the buffer
	u32* bufbuf3; // the buffer
	u32* bufbuf4; // the buffer

	dma_addr_t bufbufdmaaddr1;
	dma_addr_t bufbufdmaaddr2;
	dma_addr_t bufbufdmaaddr3;
	dma_addr_t bufbufdmaaddr4;

	bufbuf1 = dma_alloc_coherent(NULL, bufbufsize * bytesPerVal /*each u32 value has 4 bytes*/, &bufbufdmaaddr1, GFP_KERNEL);
	if (bufbuf1 == NULL) {pr_err("Unable to allocate DMA buffer 1\n");return -ENOMEM;}
	bufbuf2 = dma_alloc_coherent(NULL, bufbufsize * bytesPerVal /*each u32 value has 4 bytes*/, &bufbufdmaaddr2, GFP_KERNEL);
	if (bufbuf2 == NULL) {pr_err("Unable to allocate DMA buffer 2\n");return -ENOMEM;}
	bufbuf3 = dma_alloc_coherent(NULL, bufbufsize * bytesPerVal /*each u32 value has 4 bytes*/, &bufbufdmaaddr3, GFP_KERNEL);
	if (bufbuf3 == NULL) {pr_err("Unable to allocate DMA buffer 3\n");return -ENOMEM;}
	bufbuf4 = dma_alloc_coherent(NULL, bufbufsize * bytesPerVal /*each u32 value has 4 bytes*/, &bufbufdmaaddr4, GFP_KERNEL);
	if (bufbuf4 == NULL) {pr_err("Unable to allocate DMA buffer 4\n");return -ENOMEM;}

	for (i = 0 ; i<bufbufsize; i++)
	{
		bufbuf1[i] = outval1;
	}
	for (i = 0 ; i<bufbufsize; i++)
	{
		bufbuf2[i] = outval2;
	}
	for (i = 0 ; i<bufbufsize; i++)
	{
		bufbuf3[i] = outval3;
	}
	for (i = 0 ; i<bufbufsize; i++)
	{
		bufbuf4[i] = outval4;
	}

	printk(KERN_ALERT "Hello DMA-McBSP-send world!\n");

	/* Setting IO type & requesting McBSP */
	status = omap_mcbsp_set_io_type(mcbspID, OMAP_MCBSP_POLL_IO);  // POLL because we don't want to use IRQ and DMA will be set up when needed.
	reqstatus = omap_mcbsp_request(mcbspID);
	printk(KERN_ALERT "Setting IO type was %d. Requesting McBSP %d returned: %d \n", status, (mcbspID+1), reqstatus);
	if (!reqstatus)
	{
		/* configure McBSP */
		//omap_mcbsp_set_spi_mode(mcbspID, &ads1274_cfg_spi);
		//printk(KERN_ALERT "Configured McBSP %d for SPI mode.\n", (mcbspID+1));
		omap_mcbsp_config(mcbspID, &simon_regs);
		printk(KERN_ALERT "Configured McBSP %d registers for raw mode..\n", (mcbspID+1));

		/* start McBSP */
		omap_mcbsp_start(mcbspID);	// kernel 2.6.31 and below have only one argument here!
		printk(KERN_ALERT "Started McBSP %d. \n", (mcbspID+1));

		// Define mcbsp variable:
		getMcBSPDevice(mcbspID,&mcbsp);
		printk(KERN_ALERT "### The McBSP base address is 0x%lx\n", mcbsp->phys_base);

		// setting threshold...
		//simon_omap_mcbsp_dump_reg(mcbspID);
		//omap_mcbsp_set_rx_threshold(mcbspID, 64);
		//simon_omap_mcbsp_dump_reg(mcbspID);

		/* polled SPI mode operations */
		printk(KERN_ALERT "Now reading data from McBSP %d in SPI mode... \n", (mcbspID+1));
		status = omap_mcbsp_spi_master_recv_word_poll(mcbspID, &value32);
		printk(KERN_ALERT "Reading from McBSP %d in SPI mode returned as status: %d and as value: 0x%04x \n", (mcbspID+1), status,value32);
		printk(KERN_ALERT "Now writing data to McBSP %d in SPI mode... \n", (mcbspID+1));
		status = omap_mcbsp_spi_master_xmit_word_poll(mcbspID, mybuffer);
		printk(KERN_ALERT "Writing to McBSP %d in SPI mode returned as status: %d \n", (mcbspID+1), status);

		/* polled mcbsp i/o operations */
		printk(KERN_ALERT "Now writing data to McBSP %d (raw & polled)... \n", (mcbspID+1));
		status = omap_mcbsp_pollwrite(mcbspID, mybuffer);  // needs changes in mcbsp.c --> kernel patch & recompile
		printk(KERN_ALERT "Writing to McBSP %d (raw, polled mode) returned as status: %d \n", (mcbspID+1), status);
		printk(KERN_ALERT "Now writing 2nd data to McBSP %d (raw & polled)... \n", (mcbspID+1));
		status = omap_mcbsp_pollwrite(mcbspID, mybuffer2);  // needs changes in mcbsp.c --> kernel patch & recompile
		printk(KERN_ALERT "Writing to McBSP %d (raw, polled mode) returned as status: %d \n", (mcbspID+1), status);

		printk(KERN_ALERT "Now reading data from McBSP %d (raw & polled)... \n", (mcbspID+1));
		status = omap_mcbsp_pollread(mcbspID, &value16);  // needs changes in mcbsp.c --> kernel patch & recompile
		printk(KERN_ALERT "Reading from McBSP %d (raw, polled mode) returned as status: %d and as value 0x%x \n", (mcbspID+1), status,value32);

		/* Non-Polled operations (IRQ or DMA!) */
		/* IRQ */
		//printk(KERN_ALERT "Now writing data to McBSP %d via IRQ! \n", (mcbspID+1)); omap_mcbsp_set_io_type() must be set to OMAP_MCBSP_IRQ_IO or this will produce a NullPointer SegFault.
		//omap_mcbsp_xmit_word(mcbspID, mybuffer); // omap_mcbsp_set_io_type() must be set to OMAP_MCBSP_IRQ_IO or this will produce a NullPointer SegFault. Also check XINTM() and RINTM() in McBSPx.SPC regs/configuration structure.
		//printk(KERN_ALERT "Wrote to McBSP %d via IRQ! Return status: %d \n", (mcbspID+1), status);

		/* DMA */
		printk(KERN_ALERT "Now writing data to McBSP %d via DMA! \n", (mcbspID+1));
		status = simon_omap_mcbsp_xmit_buffer(mcbspID, bufbufdmaaddr1, bufbufsize * bytesPerVal /*becomes elem_count in http://lxr.free-electrons.com/source/arch/arm/plat-omap/dma.c#L260 */); // the dma memory must have been allocated correctly. See above.
		printk(KERN_ALERT "Wrote to McBSP %d via DMA! Return status: %d \n", (mcbspID+1), status);

		printk(KERN_ALERT "The first 40 of %d values of the transferbuffer bufbuf after transmission are: \n",bufbufsize);
		sprintf(printtemp, "trans: \n");
		for (i = 0 ; i<min(bufbufsize,40); i++)
		{
			sprintf(printtemp, "%s 0x%x,", printtemp,bufbuf1[i]);

			if ((i%16) == 15)
			{
				printk(KERN_ALERT "%s, \n",printtemp);
				sprintf(printtemp, "   ");
			}
		}
		printk(KERN_ALERT " end. \n");


		//printk(KERN_ALERT "Now reading data from McBSP %d via DMA! \n", (mcbspID+1));
		//status = simon_omap_mcbsp_recv_buffer(mcbspID, bufbufdmaaddr, bufbufsize * bytesPerVal /* = elem_count in arch/arm/plat-omap/dma.c */); // the dma memory must have been allocated correctly. See above.
		//printk(KERN_ALERT "Read from McBSP %d via DMA! Return status: %d \n", (mcbspID+1), status);
/*
		printk(KERN_ALERT "The first millions of %d values of the transferbuffer bufbuf after reception are: \n",bufbufsize);
		sprintf(printtemp, "receive: \n");
		for (i = 0 ; i<min(bufbufsize,1000000); i++)
		{
			sprintf(printtemp, "%s 0x%x,", printtemp,bufbuf[i]);

			if ((i%16) == 15)
			{
				printk(KERN_ALERT "%s \n",printtemp);
				sprintf(printtemp, "   ");
			}
		}
		printk(KERN_ALERT " end. \n");
*/
	}
	else printk(KERN_ALERT "Not attempting to continue because requesting failed.\n");



	return returnstatus;
}

void hello_exit(void)
{
	printk(KERN_ALERT "Stopping McBSP %d...", (mcbspID+1));
	omap_mcbsp_stop(mcbspID);	// kernel 2.6.31 and below have only one argument here!
	printk(KERN_ALERT "Freeing McBSP %d...", (mcbspID+1));
	omap_mcbsp_free(mcbspID);
	printk(KERN_ALERT "done.\n");

	printk(KERN_ALERT "Goodbye, McBSP world\n");
}

module_init(hello_init);
module_exit(hello_exit);
