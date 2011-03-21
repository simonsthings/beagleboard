/*
 * Skeleton Linux Kernel Module
 *
 * PUBLIC DOMAIN
 */

//done some Guttenberg here:
//http://tldp.org/LDP/lkmpg/2.6/html/lkmpg.html#AEN567
//https://sourcecode.isip.uni-luebeck.de/viewvc/beagleboard/trunk/mcbsp_ads1258_noDMA_kernelmodule/recipes/


#include <linux/module.h>
#include <linux/kernel.h>
#include <asm/uaccess.h>
//#include "kmodule_test.h"

#include <plat/mcbsp.h>
#include <mach/mcbsp.h>
#include <mach/mux.h>

#include <linux/fs.h>


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



MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Michael Fink <DePeter1@gmx.net>");


int init_module(void);
void cleanup_module(void);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

#define SUCCESS 0
#define DEVICE_NAME "biosigdev"	/* Dev name as it appears in /proc/devices   */
#define BUF_LEN 80		/* Max length of the message from the device */
#define mcbsp_base_reg OMAP34XX_MCBSP1_BASE 



/* 
 * Global variables are declared as static, so are global within the file. 
 */
static int Major;		/* Major number assigned to our device driver */
static int Device_Open = 0;	/* Is device open?  
				 * Used to prevent multiple access to device */
static char msg[BUF_LEN];	/* The msg the device will give when asked */
static char *msg_Ptr;

static struct file_operations fops = {
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release
};





int init_module(void)
{
	u16 test_read;
	u32 test_read_32;
//	struct omap_mcbsp *mcbsp;
//	getMcBSPDevice(mcbspID,&my_mcbsp);
	struct omap_mcbsp_reg_cfg my_mcbsp_confic;   
	u16 i;

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
		
		my_mcbsp_confic.pcr0 =  FSXM|CLKXM|FSXP|FSRP|CLKRP|CLKXP;//pin control register
		
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
		
	
/*			printk(KERN_ALERT "Reading_start:\n");
			omap_mcbsp_pollread(OMAP_MCBSP1, &test_read);
			printk(KERN_ALERT "reading %d. \n", test_read);
			omap_mcbsp_pollread(OMAP_MCBSP1, &test_read);
			printk(KERN_ALERT "reading %d. \n", test_read);
			omap_mcbsp_pollread(OMAP_MCBSP1, &test_read);
			printk(KERN_ALERT "reading %d. \n", test_read);

			omap_mcbsp_pollread(OMAP_MCBSP1, &test_read);
			printk(KERN_ALERT "reading %d. \n", test_read);
			omap_mcbsp_pollwrite(OMAP_MCBSP1, 0xAF);
*/
			
			//raw_reading...
			test_read_32 =__raw_readl(ioremap( mcbsp_base_reg,4));
			printk(KERN_ALERT "raw_reading %x. \n", test_read_32);
/*			test_read_32 =__raw_readl(ioremap( mcbsp_base_reg,4));
			printk(KERN_ALERT "raw_reading %x. \n", test_read_32);
			test_read_32 =__raw_readl(ioremap( mcbsp_base_reg,4));
			printk(KERN_ALERT "raw_reading %x. \n", test_read_32);
			test_read_32 =__raw_readl(ioremap( mcbsp_base_reg,4));
			printk(KERN_ALERT "raw_reading %x. \n", test_read_32);
			test_read_32 =__raw_readl(ioremap( mcbsp_base_reg,4));
			printk(KERN_ALERT "raw_reading %x. \n", test_read_32);
			test_read_32 =__raw_readl(ioremap( mcbsp_base_reg,4));
			printk(KERN_ALERT "raw_reading %x. \n", test_read_32);
*/

			//__raw_writel(0xAA0F0F,ioremap( mcbsp_base_reg+8,4));
			
/*raw_reading_loop.....			__set_current_state(TASK_INTERRUPTIBLE);

			for(i = 0; i<1000; i++)
			{
				while(!(0b10&__raw_readl(ioremap( mcbsp_base_reg+0x14,4))))  // 0x14 ersetzen durch spcr1-referenz
				{
					schedule_timeout(100);
				}
				test_read_32 =__raw_readl(ioremap( mcbsp_base_reg,4));
				printk(KERN_ALERT "%x\n", test_read_32);
			}*/


			Major = register_chrdev(0, DEVICE_NAME, &fops);

			if (Major < 0) {
			  printk(KERN_ALERT "Registering char device failed with %d\n", Major);
			  return Major;
			}

			printk(KERN_INFO "I was assigned major number %d. To talk to\n", Major);
			printk(KERN_INFO "the driver, create a dev file with\n");
			printk(KERN_INFO "'mknod /dev/%s c %d 0'.\n", DEVICE_NAME, Major);
			printk(KERN_INFO "Try various minor numbers. Try to cat and echo to\n");
			printk(KERN_INFO "the device file.\n");
			printk(KERN_INFO "Remove the device file and module when done.\n"); 
		}		
		

	return 0;
}


void cleanup_module(void)
{
	
	printk("Module cleanup called\n");
	omap_mcbsp_stop(OMAP_MCBSP1,1,1);
	omap_mcbsp_free(OMAP_MCBSP1);
	/* 
	 * Unregister the device 
	 */
	unregister_chrdev(Major, DEVICE_NAME);
	
}



/*
 * Methods
 */

/* 
 * Called when a process tries to open the device file, like
 * "cat /dev/mycharfile"
 */
static int device_open(struct inode *inode, struct file *file)
{
	static int counter = 0;

	if (Device_Open)
		return -EBUSY;

	Device_Open++;
	sprintf(msg, "I already told you %d times Hello world!\n", counter++);
	msg_Ptr = msg;
	try_module_get(THIS_MODULE);

	return SUCCESS;
}

/* 
 * Called when a process closes the device file.
 */
static int device_release(struct inode *inode, struct file *file)
{
	Device_Open--;		/* We're now ready for our next caller */

	/* 
	 * Decrement the usage count, or else once you opened the file, you'll
	 * never get get rid of the module. 
	 */
	module_put(THIS_MODULE);

	return 0;
}

/* 
 * Called when a process, which already opened the dev file, attempts to
 * read from it.
 */
static ssize_t device_read(struct file *filp,	/* see include/linux/fs.h   */
			   char *buffer,	/* buffer to fill with data */
			   size_t length,	/* length of the buffer     */
			   loff_t * offset)
{
	/*
	 * Number of bytes actually written to the buffer 
	 */
	int bytes_read = 0;
	u32 read_val = 0;
	int counter =0;

	/*
	 * If we're at the end of the message, 
	 * return 0 signifying end of file 
	 */
	if (*msg_Ptr == 0)
		return 0;

	/* 
	 * Actually put the data into the buffer 
	 */
	while(length && counter <90)
	{
// 	  counter++;
	  while(!(0b10&__raw_readl(ioremap( mcbsp_base_reg+0x14,4))))  // 0x14 ersetzen durch spcr1-referenz
	  {
		  schedule_timeout(1);
		  if(0b100&__raw_readl(ioremap( mcbsp_base_reg+0x14,4)))
		    printk(KERN_ALERT "Buffer_full_error  ---> lost data\n");
		    
	  }
	  read_val =__raw_readl(ioremap( mcbsp_base_reg,4));
	  //printk(KERN_ALERT "%x\n", read_val);

	  sprintf(msg, "%x \n", read_val);
	  msg_Ptr = msg;
	  try_module_get(THIS_MODULE);
	  
	  while (length && *msg_Ptr) 
	  {

		  /* 
		  * The buffer is in the user data segment, not the kernel 
		  * segment so "*" assignment won't work.  We have to use 
		  * put_user which copies data from the kernel data segment to
		  * the user data segment. 
		  */

		  put_user(*(msg_Ptr++), buffer++);
		  length--;
		  bytes_read++;

	  }
	}
	/* 
	 * Most read functions return the number of bytes put into the buffer
	 */
	return bytes_read;
}

/*  
 * Called when a process writes to dev file: echo "hi" > /dev/hello 
 */
static ssize_t
device_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{
	printk(KERN_ALERT "Sorry, this operation isn't supported.\n");
	return -EINVAL;
}

