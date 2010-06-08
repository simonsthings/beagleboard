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
	0x0000, //rccr;
	0x0308 //syscon
};

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

const struct omap_mcbsp_spi_cfg ads1274_cfg_spi = {
	OMAP_MCBSP_SPI_MASTER,
	OMAP_MCBSP_CLK_RISING,
	OMAP_MCBSP_CLK_RISING,
	OMAP_MCBSP_FS_ACTIVE_LOW,
	16,
	OMAP_MCBSP_CLK_STP_MODE_NO_DELAY,
	OMAP_MCBSP_WORD_32
};

int hello_init(void)
{
	int status = 3;
	int reqstatus = -4;
	int value = 0;
	int returnstatus = 0;
	u32 mybuffer = 0xABCD;
	

	printk(KERN_ALERT "Hello McBSP world!\n");

	/* Setting IO type & requesting McBSP */
	status = omap_mcbsp_set_io_type(mcbspID, OMAP_MCBSP_POLL_IO);
	reqstatus = omap_mcbsp_request(mcbspID);
	printk(KERN_ALERT "Setting IO type was %d. Requesting McBSP %d returned: %d \n", status, (mcbspID+1), reqstatus);

	if (!reqstatus)
	{
	/* configure McBSP */
	omap_mcbsp_set_spi_mode(mcbspID, &ads1274_cfg_spi);
	printk(KERN_ALERT "Configured McBSP %d for SPI mode.\n", (mcbspID+1));
	//omap_mcbsp_config(mcbspID, &mcbsp_regs);
	//printk(KERN_ALERT "Configured McBSP %d registers..\n", (mcbspID+1));

	/* start McBSP */
	omap_mcbsp_start(mcbspID);
	printk(KERN_ALERT "Started McBSP %d. \n", (mcbspID+1));

	/* polled mcbsp i/o operations */
	//status = omap_mcbsp_spi_master_xmit_word_poll(mcbspID, mybuffer);
	//printk(KERN_ALERT "Writing to McBSP %d in SPI mode returned as status: %d \n", (mcbspID+1), status);
	status = omap_mcbsp_pollwrite(mcbspID, mybuffer);  // needs changes in mcbsp.c --> kernel patch & recompile
	////omap_mcbsp_xmit_word(mcbspID, mybuffer); // currently produces segfault, probably because nothing dma-like has been set up yet?
	printk(KERN_ALERT "Writing to McBSP %d returned as status: %d \n", (mcbspID+1), status);
	}
	else printk(KERN_ALERT "Not attempting to continue because requesting failed.\n");



	return returnstatus;
}

void hello_exit(void)
{	
	printk(KERN_ALERT "Freeing McBSP %d...", (mcbspID+1));
	omap_mcbsp_free(mcbspID);
	printk(KERN_ALERT "done.\n");

	printk(KERN_ALERT "Goodbye, McBSP world\n");
}

module_init(hello_init);
module_exit(hello_exit);
