//#define DEBUG

#include <config/modversions.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/dma-mapping.h>
#include <linux/gpio.h>

//#include <linux/sched.h>
//#include <linux/workqueue.h>
//#include <linux/interrupt.h>	/* We want an interrupt */
//#include <asm/io.h>

#include <mach/mcbsp.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Simon Vogt <simonsunimail@gmail.com>");

int mcbspID = 0; /*McBSP1 => id 0*/
module_param(mcbspID, int, 0);
MODULE_PARM_DESC(mcbspID, "The McBSP to use. Starts at 0.");


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

/* ACTIVE SETTINGS:*/
static struct omap_mcbsp_reg_cfg simon_regs = {
        .spcr2 = XINTM(3),
        .spcr1 = RINTM(3),
        .rcr2  = 0,
        .rcr1  = RFRLEN1(1) | RWDLEN1(OMAP_MCBSP_WORD_16),  // frame is 1 word. word is 32 bits.
        .xcr2  = 0,
        .xcr1  = XFRLEN1(1) | XWDLEN1(OMAP_MCBSP_WORD_16),
        .srgr1 = FWID(31) | CLKGDV(50),
        .srgr2 = GSYNC | 0/*CLKSM*/ | CLKSP  | FPER(250),// | FSGM, // see pages 129 to 131 of sprufd1.pdf
        .pcr0  = FSXM | 0/*FSRM*/ | CLKXM | CLKRM | FSXP | FSRP | CLKXP | CLKRP,
        //.pcr0 = CLKXP | CLKRP,        /* mcbsp: slave */
	.xccr = DXENDLY(1) | XDMAEN ,//| XDISABLE,
	.rccr = RFULL_CYCLE | RDMAEN,// | RDISABLE,
};


int hello_init(void)
{
	int status = 3;
	int reqstatus = -4;
	//int ADS1258_CLKSEL = 0;
	u16 value16 = 0;
	u32 value32 = 0;
	int returnstatus = 0;
	u32 mybuffer = 0x5CCC333A; // 01011100110011000011001100111010
	u32 mybuffer2 = 0x53CA; // 0101 00111100 1010
	int i;

	int bufbufsize = 1 * 160; // number of array elements
	int bytesPerVal = 2; // number of bytes per array element (32bit = 4 bytes, 16bit = 2 bytes)
	u16* bufbuf;

	dma_addr_t bufbufdmaaddr;
	bufbuf = dma_alloc_coherent(NULL, bufbufsize * bytesPerVal /*each u32 value has 4 bytes*/, &bufbufdmaaddr, GFP_ATOMIC);
	if (bufbuf == NULL) 
	{
		pr_err("Unable to allocate DMA buffer\n");
		return -ENOMEM;
	}

	bufbuf[0] = 0x53CA;  // 01010011 11001010  
	bufbuf[1] = 0x5C3A;  // 01011100 00111010  
	for (i = 2 ; i<bufbufsize-1; i++)
	{
		bufbuf[i] = 0x5E7A;  // 01011110 01111010  
	}
	// so that we can see that the last word really was transmitted, it should be different:
	bufbuf[bufbufsize-1] = 0x518A;  // 01010001 10001010  

	/*
	179 
	180 **
	181  * dma_alloc_coherent - allocate consistent memory for DMA
	182  * @dev: valid struct device pointer, or NULL for ISA and EISA-like devices
	183  * @size: required memory size
	184  * @handle: bus-specific DMA address
	185  *
	186  * Allocate some uncached, unbuffered memory for a device for
	187  * performing DMA.  This function allocates pages, and will
	188  * return the CPU-viewed address, and sets @handle to be the
	189  * device-viewed address.
	190  *
	191 extern void *dma_alloc_coherent(struct device *, size_t, dma_addr_t *, gfp_t);
	192 

	1043         host->dma.nc = dma_alloc_coherent(NULL,
	1044                                           sizeof(struct msmsdcc_nc_dmadata),
	1045                                           &host->dma.nc_busaddr,
	1046                                           GFP_KERNEL);
	1047         if (host->dma.nc == NULL) {
	1048                 pr_err("Unable to allocate DMA buffer\n");
	1049                 return -ENOMEM;
	1050         }
	*/

	printk(KERN_ALERT "Hello McBSP world!\n");

	/* Do GPIO stuff */
	// requesting:
	reqstatus = gpio_request(134, "ADS1258EVM-clockselect");
	printk(KERN_ALERT "Gpio 134 (ADS1258EVM-clockselect) was requested. Return status: %d\n",reqstatus);
	// setting:
	status = gpio_direction_output(134,1);
	printk(KERN_ALERT "Setting gpio134 (ADS1258EVM-clockselect) as output, value 1=EXTERNAL clock from BB. Return status: %d\n",status);
	/* End of GPIO stuff */


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
		omap_mcbsp_start(mcbspID);	// kernel 2.6.31 and below have only one argument here!
		//omap_mcbsp_start(mcbspID,0,0); // kernel 2.6.32 and beyond require tx and rx arguments on omap_mcbsp_start() and omap_mcbsp_stop().
		printk(KERN_ALERT "Started McBSP %d. \n", (mcbspID+1));

		/* show McBSP register contents: */
		//printk(KERN_ALERT "MCBSP1_SRGR2: 0x%04x\n", __raw_readl( (*(OMAP34XX_MCBSP1_BASE + OMAP_MCBSP_REG_SRGR2)) ));

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
		status = omap_mcbsp_pollread(mcbspID, &value32);  // needs changes in mcbsp.c --> kernel patch & recompile
		printk(KERN_ALERT "Reading from McBSP %d (raw, polled mode) returned as status: %d and as value 0x%x \n", (mcbspID+1), status,value32);

		/* Non-Polled operations (IRQ or DMA!) */
		/* IRQ */
		//printk(KERN_ALERT "Now writing data to McBSP %d via IRQ! \n", (mcbspID+1)); omap_mcbsp_set_io_type() must be set to OMAP_MCBSP_IRQ_IO or this will produce a NullPointer SegFault.
		//omap_mcbsp_xmit_word(mcbspID, mybuffer); // omap_mcbsp_set_io_type() must be set to OMAP_MCBSP_IRQ_IO or this will produce a NullPointer SegFault. Also check XINTM() and RINTM() in McBSPx.SPC regs/configuration structure.
		//printk(KERN_ALERT "Wrote to McBSP %d via IRQ! Return status: %d \n", (mcbspID+1), status);

		/* DMA */
		printk(KERN_ALERT "Now writing data to McBSP %d via DMA! \n", (mcbspID+1));
		status = omap_mcbsp_xmit_buffer(mcbspID, bufbufdmaaddr, bufbufsize * bytesPerVal /*becomes elem_count in http://lxr.free-electrons.com/source/arch/arm/plat-omap/dma.c#L260 */); // currently waits forever, probably because nothing dma-like has been set up yet? Or word-length wrong?
		printk(KERN_ALERT "Wrote to McBSP %d via DMA! Return status: %d \n", (mcbspID+1), status);

		printk(KERN_ALERT "The first 40 of %d values of the transferbuffer bufbuf after transmission are: \n",bufbufsize);
		for (i = 0 ; i<min(bufbufsize,40); i++)
		{
			printk(KERN_ALERT " 0x%x,", bufbuf[i]);
			if ((i%10) == 9)
			{
				printk(KERN_ALERT ", \n");
			}
		}
		printk(KERN_ALERT " end. \n");


		printk(KERN_ALERT "Now reading data from McBSP %d via DMA! \n", (mcbspID+1));
		status = omap_mcbsp_recv_buffer(mcbspID, bufbufdmaaddr, bufbufsize * bytesPerVal /*becomes elem_count in http://lxr.free-electrons.com/source/arch/arm/plat-omap/dma.c#L260 */); // currently waits forever, probably because nothing dma-like has been set up yet? Or word-length wrong?
		printk(KERN_ALERT "Read from McBSP %d via DMA! Return status: %d \n", (mcbspID+1), status);

		printk(KERN_ALERT "The first 160 of %d values of the transferbuffer bufbuf after reception are: \n",bufbufsize);
		for (i = 0 ; i<min(bufbufsize,160); i=i+2)
		{
			printk(KERN_ALERT " 0x%x 00 %x,", bufbuf[i+1],bufbuf[i]);
			if ((i%8) == 6)
			{
				printk(KERN_ALERT ", \n");
			}
		}
		printk(KERN_ALERT " end. \n");
		

		

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
