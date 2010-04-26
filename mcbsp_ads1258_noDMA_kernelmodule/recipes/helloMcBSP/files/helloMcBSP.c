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

const struct omap_mcbsp_spi_cfg ads1274_cfg_spi = {
	OMAP_MCBSP_SPI_MASTER,
	OMAP_MCBSP_CLK_RISING,
	OMAP_MCBSP_CLK_RISING,
	OMAP_MCBSP_FS_ACTIVE_LOW,
	8,
	OMAP_MCBSP_CLK_STP_MODE_DELAY,
	OMAP_MCBSP_WORD_16
};

int hello_init(void)
{
	int status = 3;
	int reqstatus = -4;
	int value = 0;
	int returnstatus = 0;
	u16 mybuffer = 4657;
	

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

	/* start McBSP */
	omap_mcbsp_start(mcbspID);
	printk(KERN_ALERT "Started McBSP %d. \n", (mcbspID+1));

	/* polled mcbsp i/o operations */
	status = omap_mcbsp_pollwrite(mcbspID, mybuffer);
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
