/*
 * Skeleton Linux Kernel Module
 *
 * PUBLIC DOMAIN
 */

#include <linux/module.h>
#include <linux/kernel.h>

#include <plat/mcbsp.h>
#include <mach/mcbsp.h>
#include <mach/mux.h>


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
#include <plat/mcbsp.h>

#define mcbsp_base_reg OMAP34XX_MCBSP1_BASE 

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Michael Fink <DePeter1@gmx.net>");



/*void omap_mcbsp_write(void __iomem *io_base, u16 reg, u32 val)
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

//#define id_to_mcbsp_ptr(id)		mcbsp_ptr[id];

//0x48074000

static void my_omap_mcbsp_dump_reg(u8 id)
{
	//printk(KERN_ALERT "**** McBSP%d regs ****\n", mcbsp_base_reg);
	printk(KERN_ALERT "DRR2:  0x%04x\n",
			OMAP_MCBSP_READ(mcbsp_base_reg, DRR2));
	printk(KERN_ALERT "DRR1:  0x%04x\n",
			OMAP_MCBSP_READ(mcbsp_base_reg, DRR1));
	printk(KERN_ALERT "DXR2:  0x%04x\n",
			OMAP_MCBSP_READ(mcbsp_base_reg, DXR2));
	printk(KERN_ALERT "DXR1:  0x%04x\n",
			OMAP_MCBSP_READ(mcbsp_base_reg, DXR1));
	printk(KERN_ALERT "SPCR2: 0x%04x\n",
			OMAP_MCBSP_READ(mcbsp_base_reg, SPCR2));
	printk(KERN_ALERT "SPCR1: 0x%04x\n",
			OMAP_MCBSP_READ(mcbsp_base_reg, SPCR1));
	printk(KERN_ALERT "RCR2:  0x%04x\n",
			OMAP_MCBSP_READ(mcbsp_base_reg, RCR2));
	printk(KERN_ALERT "RCR1:  0x%04x\n",
			OMAP_MCBSP_READ(mcbsp_base_reg, RCR1));
	printk(KERN_ALERT "XCR2:  0x%04x\n",
			OMAP_MCBSP_READ(mcbsp_base_reg, XCR2));
	printk(KERN_ALERT "XCR1:  0x%04x\n",
			OMAP_MCBSP_READ(mcbsp_base_reg, XCR1));
	printk(KERN_ALERT "SRGR2: 0x%04x\n",
			OMAP_MCBSP_READ(mcbsp_base_reg, SRGR2));
	printk(KERN_ALERT "SRGR1: 0x%04x\n",
			OMAP_MCBSP_READ(mcbsp_base_reg, SRGR1));
	printk(KERN_ALERT "PCR0:  0x%04x\n",
			OMAP_MCBSP_READ(mcbsp_base_reg, PCR0));
	printk(KERN_ALERT "***********************\n");
}
*/


int init_module(void)
{
	u16 test_read;
	u32 test_read_32;
//	struct omap_mcbsp *mcbsp;
//	getMcBSPDevice(mcbspID,&my_mcbsp);
	struct omap_mcbsp_reg_cfg my_mcbsp_confic;   

	test_read =0;

	printk("Module init called\n");


	
		my_mcbsp_confic.spcr2 =  /* FRST | GRST |*/ XINTM(0x0) ; //serial-port control register RST?????
		my_mcbsp_confic.spcr1 = RJUST(0x0)/*| DXENA*/ |RINTM(0x0) /*| RRST*/;  //rst?? dxena
		
		my_mcbsp_confic.rcr2 = 0 ; //RFRLEN2(OMAP_MCBSP_WORD_24) |RWDLEN2(OMAP_MCBSP_WORD_24) | RDATDLY(1);  //receive control register 
		my_mcbsp_confic.rcr1 =  RFRLEN2(0) |RWDLEN2(OMAP_MCBSP_WORD_24);
		
		my_mcbsp_confic.xcr2 = 0 ; //XFRLEN2(OMAP_MCBSP_WORD_24) | XWDLEN2(OMAP_MCBSP_WORD_24) | XDATDLY(1);  //transmit control register 
		my_mcbsp_confic.xcr1 = XFRLEN1(0) | XWDLEN1(OMAP_MCBSP_WORD_24);
		
		my_mcbsp_confic.srgr2 = FPER(24);// seite 3027 fper????? //srg-register, what ever that means
		my_mcbsp_confic.srgr1 = FWID(23) | CLKGDV(40);   //FWID?????
		
		my_mcbsp_confic.mcr2=0x0;  //multichan-register 
		my_mcbsp_confic.mcr1=0x0;
		
		my_mcbsp_confic.pcr0 =  FSXM|CLKXM|FSXP|FSRP|CLKRP;//pin control register
		
		my_mcbsp_confic.rcerc=0; //multichan-options... ignore? 
		my_mcbsp_confic.rcerd=0;//multichan-options... ignore? 
		my_mcbsp_confic.xcerc=0;//multichan-options... ignore? 
		my_mcbsp_confic.xcerd=0;//multichan-options... ignore? 
		my_mcbsp_confic.rcere=0;//multichan-options... ignore? 
		my_mcbsp_confic.rcerf=0;//multichan-options... ignore? 
		my_mcbsp_confic.xcere=0;//multichan-options... ignore? 
		my_mcbsp_confic.xcerf=0;//multichan-options... ignore? 
		my_mcbsp_confic.rcerg=0;//multichan-options... ignore? 
		my_mcbsp_confic.rcerh=0;//multichan-options... ignore? 
		my_mcbsp_confic.xcerg=0;//multichan-options... ignore? 
		my_mcbsp_confic.xcerh=0;//multichan-options... ignore? 
		
		my_mcbsp_confic.xccr = DXENDLY(1) | XDMAEN;
		my_mcbsp_confic.rccr = RFULL_CYCLE | RDMAEN; 
		
		
		printk(KERN_ALERT "set-io-start...\n");
		test_read = omap_mcbsp_set_io_type(OMAP_MCBSP1, OMAP_MCBSP_POLL_IO); 
		printk(KERN_ALERT "set-io-started respnce: %d. \n", test_read);

		printk(KERN_ALERT "request-start...\n");
		test_read = omap_mcbsp_request(OMAP_MCBSP1);
		printk(KERN_ALERT "reqeust-started responce: %d. \n", test_read);
		
		if(test_read==0)
		{
		
			printk(KERN_ALERT "runing config... \n");
			/*test_read =*/ omap_mcbsp_config(OMAP_MCBSP1, &my_mcbsp_confic);
			printk(KERN_ALERT "result-config: %d. \n", test_read);

			printk(KERN_ALERT "mcbsp_start...\n");
			/*test_read =*/ omap_mcbsp_start(OMAP_MCBSP1,1,1);
			printk(KERN_ALERT "mcbsp_start: %d. \n", test_read);
		
	//		my_omap_mcbsp_dump_reg(OMAP_MCBSP1);
	//		mcbsp = id_to_mcbsp_ptr(id); 
	//		MCBSP_WRITE(mcbsp, DXR1, 0xAF0F);

		
		
			printk(KERN_ALERT "Reading_start:\n");
			omap_mcbsp_pollread(OMAP_MCBSP1, &test_read);
			printk(KERN_ALERT "reading %d. \n", test_read);
			omap_mcbsp_pollread(OMAP_MCBSP1, &test_read);
			printk(KERN_ALERT "reading %d. \n", test_read);
			omap_mcbsp_pollread(OMAP_MCBSP1, &test_read);
			printk(KERN_ALERT "reading %d. \n", test_read);
			//for(;;)
			//{
			  omap_mcbsp_pollread(OMAP_MCBSP1, &test_read);
			  printk(KERN_ALERT "reading %d. \n", test_read);
			  
			  omap_mcbsp_pollwrite(OMAP_MCBSP1, 0xAF);

			//printk(KERN_ALERT "reading %d. \n", *OMAP34XX_MCBSP1_BASE ); //think about that...
			//}
			//my_omap_mcbsp_dump_reg(OMAP_MCBSP1);
			test_read_32 =__raw_readl(ioremap( mcbsp_base_reg,4));
			printk(KERN_ALERT "raw_reading %x. \n", test_read_32);
			test_read_32 =__raw_readl(ioremap( mcbsp_base_reg,4));
			printk(KERN_ALERT "raw_reading %x. \n", test_read_32);
			test_read_32 =__raw_readl(ioremap( mcbsp_base_reg,4));
			printk(KERN_ALERT "raw_reading %x. \n", test_read_32);
			test_read_32 =__raw_readl(ioremap( mcbsp_base_reg,4));
			printk(KERN_ALERT "raw_reading %x. \n", test_read_32);
			test_read_32 =__raw_readl(ioremap( mcbsp_base_reg,4));
			printk(KERN_ALERT "raw_reading %x. \n", test_read_32);
			test_read_32 =__raw_readl(ioremap( mcbsp_base_reg,4));
			printk(KERN_ALERT "raw_reading %x. \n", test_read_32);

		}		
		

	return 0;
}


void cleanup_module(void)
{
	printk("Module cleanup called\n");
	omap_mcbsp_stop(OMAP_MCBSP1,1,1);
	omap_mcbsp_free(OMAP_MCBSP1);
	
	
}
