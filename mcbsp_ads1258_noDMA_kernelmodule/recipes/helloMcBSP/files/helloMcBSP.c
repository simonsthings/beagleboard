#define DEBUG

#include <linux/init.h>
#include <linux/module.h>
#include <linux/gpio.h>

#include <linux/sched.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>	/* We want an interrupt */
#include <asm/io.h>

#include <mach/mcbsp.h>

MODULE_LICENSE("Dual BSD/GPL");

int mcbspID = 0; /*McBSP1 => id 0*/
module_param(mcbspID, int, 0);
MODULE_PARM_DESC(mcbspID, "The McBSP to use. Starts at 0.");


struct omap_mcbsp_reg_cfg ads1274_cfg = {
	0x0000 | FREE, //spcr2
 	0x0000, //spcr1
	0x0000, //rcr2 RPHASE (bit 15) | RFRLEN2 (14 - 8) | RWDLEN2 (7-5) |RREVERSE(4-3) | RSVD 2 | RDATDLY
 	0x0038, //rcr1  -- 24 bits 4 words
	0x0000, //xcr2
	0x0038, //xcr1
 	GSYNC | FSGM | FPER(200), //srgr2 -- sample rate generater
	FWID(10) | CLKGDV(8), //srgr1
 	0x0000, //mcr2
	0x0000, //mc1;
	CLKXM | CLKRM | FSXM | FSRP | FSXP , //pcr0;
 	0x0000, //rcerc;
	0x0000, //rcerd;
	0x0000, //xcerc;
 	0x0000, //xcerd;
	0x0000, // rcere;
	0x0000, // rcerf;
 	0x0000, // xcere;
	0x0000, // xcerf;
	0x0000, //rcerg;
 	0x0000, // rcerh;
	0x0000, // xcerg;
	0x0000, //xcerh;
 	0x0000, // xccr;
	0x0000 //rccr;
	//0x0308 //syscon
};

/*original from http://old.nabble.com/ADC--McBSP-advice-and-questions-about-writing-a-driver.-td26889185.html */
static struct omap_mcbsp_reg_cfg mcbsp_regs = {
        .spcr2 = FREE,
        .spcr1 = RINTM(0),
        .rcr2  = RFRLEN2(OMAP_MCBSP_WORD_8) |RWDLEN2(OMAP_MCBSP_WORD_8) | RDATDLY(1),
        .rcr1  = RFRLEN1(OMAP_MCBSP_WORD_8) | RWDLEN1(OMAP_MCBSP_WORD_8),
        .xcr2  = XFRLEN2(OMAP_MCBSP_WORD_8) | XWDLEN2(OMAP_MCBSP_WORD_8) | XDATDLY(1),
        .xcr1  = XFRLEN1(OMAP_MCBSP_WORD_8) | XWDLEN1(OMAP_MCBSP_WORD_8),
        .srgr1 = FWID(15) | CLKGDV(50),
        .srgr2 = CLKSM | CLKSP  | FPER(255), //| FSGM
        .pcr0  = FSXM | CLKXM | CLKRM | FSRM | FSXP | FSRP | CLKXP | CLKRP,
        //.pcr0 = CLKXP | CLKRP,        /* mcbsp: slave */
};

static struct omap_mcbsp_reg_cfg simon_regs = {
        .spcr2 = XINTM(3),
        .spcr1 = RINTM(3),
        .rcr2  = 0,
        .rcr1  = RFRLEN1(0) | RWDLEN1(OMAP_MCBSP_WORD_32),  // frame is 1 word. word is 32 bits.
        .xcr2  = 0,
        .xcr1  = XFRLEN1(0) | XWDLEN1(OMAP_MCBSP_WORD_32),
        .srgr1 = FWID(31) | CLKGDV(50),
        .srgr2 = 0 | CLKSM | CLKSP  | FPER(4095),// | FSGM, // see pages 129 to 131 of sprufd1.pdf
        .pcr0  = FSXM | CLKXM | CLKRM | FSRM | FSXP | FSRP | CLKXP | CLKRP,
        //.pcr0 = CLKXP | CLKRP,        /* mcbsp: slave */
	//.xccr = EXTCLKGATE | XDMAEN | XDISABLE,
	//.rccr = RDMAEN | RDISABLE,
};

static struct omap_mcbsp_reg_cfg spimode_regs = {
        .spcr2 = 0,
        .spcr1 = RINTM(0),
        .rcr2  = RFRLEN2(OMAP_MCBSP_WORD_16) | RWDLEN2(OMAP_MCBSP_WORD_16) | RDATDLY(1),
        .rcr1  = RFRLEN1(OMAP_MCBSP_WORD_16) | RWDLEN1(OMAP_MCBSP_WORD_16),
        .xcr2  = XFRLEN2(OMAP_MCBSP_WORD_16) | XWDLEN2(OMAP_MCBSP_WORD_16) | XDATDLY(1),
        .xcr1  = XFRLEN1(OMAP_MCBSP_WORD_16) | XWDLEN1(OMAP_MCBSP_WORD_16),
        .srgr1 = FWID(15) | CLKGDV(50),
        .srgr2 = 0 | /*CLKSM |*/ CLKSP  | FPER(255),// | FSGM, // see pages 129 to 131 of sprufd1.pdf
        .pcr0  = FSXM | CLKXM | CLKRM | FSRM | FSXP | FSRP | CLKXP | CLKRP,
        //.pcr0 = CLKXP | CLKRP,        /* mcbsp: slave */
	//.xccr = XDISABLE,
};

const struct omap_mcbsp_spi_cfg ads1274_cfg_spi = {
	OMAP_MCBSP_SPI_MASTER,
	OMAP_MCBSP_CLK_RISING,
	OMAP_MCBSP_CLK_RISING,
	OMAP_MCBSP_FS_ACTIVE_LOW,
	16,
	OMAP_MCBSP_CLK_STP_MODE_DELAY,
	OMAP_MCBSP_WORD_32
};

int omap_mcbsp_read_simon(void __iomem *io_base, u16 reg)
{
	printk(KERN_ALERT "reading four bytes from address!\n");
	return __raw_readl(io_base + reg);
}

int hello_init(void)
{
	int status = 3;
	int reqstatus = -4;
	int value = 0;
	int returnstatus = 0;
	u32 mybuffer = 0x5CCC333A; // 01011100110011000011001100111010
	u32 mybuffer2 = 0x53CA; // 0101 00111100 1010
	dma_addr_t bufferbuffer[] = {0x432,0x6543}; 


	printk(KERN_ALERT "Hello McBSP world!\n");

	/* Setting IO type & requesting McBSP */
	status = omap_mcbsp_set_io_type(mcbspID, OMAP_MCBSP_POLL_IO);  // POLL because we don't want to use IRQ and DMA will be set up when needed.
	
	reqstatus = omap_mcbsp_request(mcbspID);
	printk(KERN_ALERT "Setting IO type was %d. Requesting McBSP %d returned: %d \n", status, (mcbspID+1), reqstatus);

	if (!reqstatus)
	{
		//status = omap_mcbsp_set_io_type(mcbspID, OMAP_MCBSP_POLL_IO);

		/* configure McBSP */
		//omap_mcbsp_set_spi_mode(mcbspID, &ads1274_cfg_spi);
		//printk(KERN_ALERT "Configured McBSP %d for SPI mode.\n", (mcbspID+1));
		omap_mcbsp_config(mcbspID, &simon_regs);
		printk(KERN_ALERT "Configured McBSP %d registers for raw mode..\n", (mcbspID+1));

		/* start McBSP */
		omap_mcbsp_start(mcbspID);
		printk(KERN_ALERT "Started McBSP %d. \n", (mcbspID+1));

		//value = reg32r(OMAP34XX_MCBSP1_BASE, OMAP_MCBSP_REG_SPCR2);
		//printk(KERN_ALERT "Value of McBSP register SPCR2: %d \n", value);
	
		/* show McBSP register contents: */
		//printk(KERN_ALERT "MCBSP1_SRGR2: 0x%04x\n", __raw_readl( (*(OMAP34XX_MCBSP1_BASE + OMAP_MCBSP_REG_SRGR2)) ));

		/* polled SPI mode operations */
		printk(KERN_ALERT "Now reading data from McBSP %d in SPI mode... \n", (mcbspID+1));
		status = omap_mcbsp_spi_master_recv_word_poll(mcbspID, &value);
		printk(KERN_ALERT "Reading from McBSP %d in SPI mode returned as status: %d and as value: 0x%04x \n", (mcbspID+1), status,value);
		printk(KERN_ALERT "Now writing data to McBSP %d in SPI mode... \n", (mcbspID+1));
		status = omap_mcbsp_spi_master_xmit_word_poll(mcbspID, mybuffer);
		printk(KERN_ALERT "Writing to McBSP %d in SPI mode returned as status: %d \n", (mcbspID+1), status);

		/* polled mcbsp i/o operations */
		printk(KERN_ALERT "Now writing data to McBSP (raw & polled) %d... \n", (mcbspID+1));
		status = omap_mcbsp_pollwrite(mcbspID, mybuffer);  // needs changes in mcbsp.c --> kernel patch & recompile
		printk(KERN_ALERT "Writing to McBSP %d (raw, polled mode) returned as status: %d \n", (mcbspID+1), status);
		printk(KERN_ALERT "Now writing 2nd data to McBSP (raw & polled) %d... \n", (mcbspID+1));
		status = omap_mcbsp_pollwrite(mcbspID, mybuffer2);  // needs changes in mcbsp.c --> kernel patch & recompile
		printk(KERN_ALERT "Writing to McBSP %d (raw, polled mode) returned as status: %d \n", (mcbspID+1), status);
		printk(KERN_ALERT "Now reading data from McBSP (raw & polled) %d... \n", (mcbspID+1));
		status = omap_mcbsp_pollread(mcbspID, &mybuffer2);  // needs changes in mcbsp.c --> kernel patch & recompile
		printk(KERN_ALERT "Reading from McBSP %d (raw, polled mode) returned as status: %d and as value 0x%x \n", (mcbspID+1), status,mybuffer2);

		/* Non-Polled operations (IRQ or DMA!) */
		/* IRQ */
		//printk(KERN_ALERT "Now writing data to McBSP %d via IRQ! \n", (mcbspID+1));
		//omap_mcbsp_xmit_word(mcbspID, mybuffer); // currently produces segfault, probably because nothing dma-like has been set up yet? Yes, the segfault was because i was not using OMAP_MCBSP_IRQ_IO in omap_mcbsp_set_io_type(). Still, it now either fails on start (when taking mcbsp out of reset, sprc2_XINTM=0) or on dma/irq transfer (sprc2_XINTM=1)
		//printk(KERN_ALERT "Wrote to McBSP %d via IRQ! Return status: %d \n", (mcbspID+1), status);

		/* DMA */
		printk(KERN_ALERT "Now writing data to McBSP %d via DMA! \n", (mcbspID+1));
		omap_mcbsp_xmit_buffer(mcbspID, bufferbuffer, 2); // currently produces segfault, probably because nothing dma-like has been set up yet? Yes, the segfault was because i was not using OMAP_MCBSP_IRQ_IO in omap_mcbsp_set_io_type(). Still, it now either fails on start (when taking mcbsp out of reset, sprc2_XINTM=0) or on dma/irq transfer (sprc2_XINTM=1)
		printk(KERN_ALERT "Wrote to McBSP %d via DMA! Return status: %d \n", (mcbspID+1), status);
	}
	else printk(KERN_ALERT "Not attempting to continue because requesting failed.\n");



	return returnstatus;
}

void hello_exit(void)
{	
	printk(KERN_ALERT "Stopping McBSP %d...", (mcbspID+1));
	omap_mcbsp_stop(mcbspID);
	printk(KERN_ALERT "Freeing McBSP %d...", (mcbspID+1));
	omap_mcbsp_free(mcbspID);
	printk(KERN_ALERT "done.\n");

	printk(KERN_ALERT "Goodbye, McBSP world\n");
}

module_init(hello_init);
module_exit(hello_exit);
