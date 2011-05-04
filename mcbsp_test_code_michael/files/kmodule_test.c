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


#include <linux/dma-mapping.h>
#include <mach/dma.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Michael Fink <DePeter1@gmx.net>");


int init_module(void);
void cleanup_module(void);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

void receive_function_fpga( u16 * data_puffer,bool* t_err);
//u32 save_read(bool* t_err);
u32 save_DMA_read(bool* t_err);

int dma_init(unsigned int id, dma_addr_t buffer1dma, u32* buffer1kernel, dma_addr_t buffer2dma, u32* buffer2kernel, dma_addr_t buffer3dma, u32* buffer3kernel, unsigned int length);
static void my_mcbsp_rx_dma_buf_callback(int lch, u16 ch_status, void *data);

//char_dev-Defines
#define SUCCESS 0
#define DEVICE_NAME "biosigdev"	/* Dev name as it appears in /proc/devices   */
#define BUF_LEN 500		/* Max length of the message from the device */

//DMA-Defines
#define mcbsp_base_reg OMAP34XX_MCBSP1_BASE
#define DMASIZE 3000
#define DMABUFBYTES DMASIZE*4
#define DMABUFSIZE DMASIZE



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



int buf1_dmachannel;
int buf2_dmachannel;
int buf3_dmachannel;

static bool overflow_err; //used to prevent log spamming...


//pointers for comunication betwen callback function and read function
volatile u32 * buffer_read_ptr;
volatile u32 * buffer_wait1_ptr;
volatile u32 * buffer_wait2_ptr;



int init_module(void)
{
  

	u16 test_read;
	u32 test_read_32;
	//struct omap_mcbsp *mcbsp;
	//getMcBSPDevice(mcbspID,&my_mcbsp);
	struct omap_mcbsp_reg_cfg my_mcbsp_confic;   

	
	u32* dma_buf_kspace1; // the DMA buffer for DMA channel 1
	u32* dma_buf_kspace2; // the DMA buffer for DMA channel 2
	u32* dma_buf_kspace3; // the DMA buffer for DMA channel 3

	/* The pointers to the same DMA buffers for use only by DMA controller: */
	dma_addr_t dma_buf_dma_space1;
	dma_addr_t dma_buf_dma_space2;
	dma_addr_t dma_buf_dma_space3;
	
	printk("Module init called\n");
	buffer_read_ptr = NULL;
	buffer_wait1_ptr = NULL;
	buffer_wait2_ptr = NULL;
	test_read =0;
	bool overflow_err =0;

  //set the configuration of MCBSP-Port
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
	printk(KERN_ALERT "set-io-started responce: %d. \n", test_read);

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

		//raw_reading...
		test_read_32 =__raw_readl(ioremap( mcbsp_base_reg,4));
		printk(KERN_ALERT "raw_reading %x. \n", test_read_32);


		//__raw_writel(0x000000,ioremap( mcbsp_base_reg+8,4));
		
  //		__set_current_state(TASK_INTERRUPTIBLE);



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
		
		
		/* Allocate memory space for the three buffers and assign the 6 pointers: */
		dma_buf_kspace1 = dma_alloc_coherent(NULL, DMABUFBYTES, &dma_buf_dma_space1, GFP_KERNEL);
		if (dma_buf_kspace1 == NULL) {pr_err("Unable to allocate DMA buffer 1\n");return -ENOMEM;}
		dma_buf_kspace2 = dma_alloc_coherent(NULL, DMABUFBYTES, &dma_buf_dma_space2, GFP_KERNEL);
		if (dma_buf_kspace2 == NULL) {pr_err("Unable to allocate DMA buffer 1\n");return -ENOMEM;}
		dma_buf_kspace3 = dma_alloc_coherent(NULL, DMABUFBYTES, &dma_buf_dma_space3, GFP_KERNEL);
		if (dma_buf_kspace3 == NULL) {pr_err("Unable to allocate DMA buffer 1\n");return -ENOMEM;}
		
		test_read_32 =__raw_readl(ioremap( mcbsp_base_reg,4));
		printk(KERN_ALERT "raw_reading %x. \n", test_read_32);			
		test_read_32 =__raw_readl(ioremap( mcbsp_base_reg,4));
		printk(KERN_ALERT "raw_reading %x. \n", test_read_32);			
		test_read_32 =__raw_readl(ioremap( mcbsp_base_reg,4));
		printk(KERN_ALERT "raw_reading %x. \n", test_read_32);
		
		
		
		printk(KERN_INFO "Calling DMA_init\n"); 
		dma_init(0/*id*/, dma_buf_dma_space1, dma_buf_kspace1,dma_buf_dma_space2, dma_buf_kspace2, dma_buf_dma_space3, dma_buf_kspace3, DMABUFBYTES/4 ); 
		printk(KERN_INFO "DMA is running?\n"); 
		
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

	if (Device_Open)
		return -EBUSY;

	Device_Open++;
	__raw_writel(0xF4FFFF,ioremap( mcbsp_base_reg+8,4));
//	sprintf(msg, "I already told you %d times Hello world!\n", counter++);
//	msg_Ptr = msg;
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
	u16 test_buffer[72];
	u16 i=0;
	bool t_err= false;
	/*
	 * Number of bytes actually written to the buffer 
	 */
	int bytes_read = 0;
	/*
	 * If we're at the end of the message, 
	 * return 0 signifying end of file 
	 */

	/* 
	 * Actually put the data into the buffer 
	 */

	    sprintf(msg, "\n");
	    receive_function_fpga(test_buffer,&t_err);
	    if(!t_err)
	      for(i=0;i<72;i++)
	      {
		sprintf(msg + strlen(msg)-1, "%x,\n",test_buffer[i]);
		//printk(KERN_ALERT "%x\n",test_buffer[i] );
	      }
	  
	  
	  
	    msg_Ptr = msg;
	    try_module_get(THIS_MODULE);
	    
	    while (*msg_Ptr && length) 
	    {
		    put_user(*(msg_Ptr++), buffer++);
		    length--;
		    bytes_read++;	    
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





void receive_function_fpga( u16 * data_puffer, bool* t_err)
{
  u32 read_val;
  u16 * data_puffer_ptr;
  
  u16 p_offset=0;

  //find chan-1 and save it
  do
  {
    read_val=save_DMA_read(t_err);
  }while((read_val&0b11111110)!=0); //ersten kanal finden

  do
  {
    data_puffer_ptr = data_puffer+p_offset;
    p_offset++;

    
    do
    {
      *data_puffer_ptr= (read_val&0xFFFF00)>>8;
      data_puffer_ptr+=8;
      read_val=save_DMA_read(t_err);
      
    }while((read_val&0b11111110)!=p_offset<<5&&data_puffer_ptr <=data_puffer+72); //zweiten kanal finden
  }while(p_offset <=7);

}



//mainly inspired by Simon!
int dma_init(unsigned int id, dma_addr_t buffer1dma, u32* buffer1kernel, dma_addr_t buffer2dma, u32* buffer2kernel, dma_addr_t buffer3dma, u32* buffer3kernel, unsigned int length)
{
	//struct omap_mcbsp *mcbsp;
	//int dma_rx_ch;
//	int src_port = 0;
//	int dest_port = 0;
//	int sync_dev = 0;
	int status = 3; // dummy value
	int deviceRequestlineForDmaChannelsync = 0;
	char printtemp[500];
	int i;

	struct omap_dma_channel_params *dmaparams1;
	//int buf1_dmachannel;
	struct omap_dma_channel_params *dmaparams2;
	//int buf2_dmachannel;
	struct omap_dma_channel_params *dmaparams3;
	//int buf3_dmachannel;

	dmaparams1 = kzalloc(sizeof(struct omap_dma_channel_params), GFP_KERNEL);
	if (!dmaparams1) {
		return -ENOMEM;
	}
	dmaparams2 = kzalloc(sizeof(struct omap_dma_channel_params), GFP_KERNEL);
	if (!dmaparams2) {
		return -ENOMEM;
	}
	dmaparams3 = kzalloc(sizeof(struct omap_dma_channel_params), GFP_KERNEL);
	if (!dmaparams3) {
		return -ENOMEM;
	}


	printk(KERN_INFO "Doing DMA_config\n"); 

	deviceRequestlineForDmaChannelsync = OMAP24XX_DMA_MCBSP1_RX; // RX

	dmaparams1->data_type=OMAP_DMA_DATA_TYPE_S32;		/* data type 8,16,32 */
	dmaparams1->elem_count=length;		/* number of elements in a frame */
	dmaparams1->frame_count=1;	/* number of frames in a element */

	dmaparams1->src_port=0;		/* Only on OMAP1 REVISIT: Is this needed? */
	dmaparams1->src_amode=OMAP_DMA_AMODE_CONSTANT;		/* constant, post increment, indexed,double indexed */
	dmaparams1->src_start=  (mcbsp_base_reg);		/* source address : physical */
	dmaparams1->src_ei=0;		/* source element index */
	dmaparams1->src_fi=0;		/* source frame index */

	dmaparams1->dst_port=0;		/* Only on OMAP1 REVISIT: Is this needed? */
	dmaparams1->dst_amode=OMAP_DMA_AMODE_POST_INC;		/* constant, post increment, indexed,double indexed */
	dmaparams1->dst_start=buffer1dma;		/* source address : physical */
	dmaparams1->dst_ei=0;		/* source element index */
	dmaparams1->dst_fi=0;		/* source frame index */

	dmaparams1->trigger=deviceRequestlineForDmaChannelsync;		/* trigger attached if the channel is synchronized */
	dmaparams1->sync_mode=OMAP_DMA_SYNC_ELEMENT;		/* sycn on element, frame , block or packet */
	dmaparams1->src_or_dst_synch=1;	/* source synch(1) or destination synch(0) */

	dmaparams1->ie=0;			/* interrupt enabled */

	dmaparams1->read_prio=0;/* read priority */
	dmaparams1->write_prio=0;/* write priority */

	dmaparams1->burst_mode=OMAP_DMA_DATA_BURST_DIS; /* Burst mode 4/8/16 words */


	dmaparams2->data_type=OMAP_DMA_DATA_TYPE_S32;		/* data type 8,16,32 */
	dmaparams2->elem_count=length;		/* number of elements in a frame */
	dmaparams2->frame_count=1;	/* number of frames in a element */

	dmaparams2->src_port=0;		/* Only on OMAP1 REVISIT: Is this needed? */
	dmaparams2->src_amode=OMAP_DMA_AMODE_CONSTANT;		/* constant, post increment, indexed,double indexed */
	dmaparams2->src_start= (mcbsp_base_reg);		/* source address : physical */
	dmaparams2->src_ei=0;		/* source element index */
	dmaparams2->src_fi=0;		/* source frame index */

	dmaparams2->dst_port=0;		/* Only on OMAP1 REVISIT: Is this needed? */
	dmaparams2->dst_amode=OMAP_DMA_AMODE_POST_INC;		/* constant, post increment, indexed,double indexed */
	dmaparams2->dst_start=buffer2dma;		/* source address : physical */
	dmaparams2->dst_ei=0;		/* source element index */
	dmaparams2->dst_fi=0;		/* source frame index */

	dmaparams2->trigger=deviceRequestlineForDmaChannelsync;		/* trigger attached if the channel is synchronized */
	dmaparams2->sync_mode=OMAP_DMA_SYNC_ELEMENT;		/* sycn on element, frame , block or packet */
	dmaparams2->src_or_dst_synch=1;	/* source synch(1) or destination synch(0) */

	dmaparams2->ie=0;			/* interrupt enabled */

	dmaparams2->read_prio=0;/* read priority */
	dmaparams2->write_prio=0;/* write priority */

	dmaparams2->burst_mode=OMAP_DMA_DATA_BURST_DIS; /* Burst mode 4/8/16 words */


	dmaparams3->data_type=OMAP_DMA_DATA_TYPE_S32;		/* data type 8,16,32 */
	dmaparams3->elem_count=length;		/* number of elements in a frame */
	dmaparams3->frame_count=1;	/* number of frames in a element */

	dmaparams3->src_port=0;		/* Only on OMAP1 REVISIT: Is this needed? */
	dmaparams3->src_amode=OMAP_DMA_AMODE_CONSTANT;		/* constant, post increment, indexed,double indexed */
	dmaparams3->src_start= (mcbsp_base_reg);		/* source address : physical */
	dmaparams3->src_ei=0;		/* source element index */
	dmaparams3->src_fi=0;		/* source frame index */

	dmaparams3->dst_port=0;		/* Only on OMAP1 REVISIT: Is this needed? */
	dmaparams3->dst_amode=OMAP_DMA_AMODE_POST_INC;		/* constant, post increment, indexed,double indexed */
	dmaparams3->dst_start=buffer3dma;		/* source address : physical */
	dmaparams3->dst_ei=0;		/* source element index */
	dmaparams3->dst_fi=0;		/* source frame index */

	dmaparams3->trigger=deviceRequestlineForDmaChannelsync;		/* trigger attached if the channel is synchronized */
	dmaparams3->sync_mode=OMAP_DMA_SYNC_ELEMENT;		/* sycn on element, frame , block or packet */
	dmaparams3->src_or_dst_synch=1;	/* source synch(1) or destination synch(0) */

	dmaparams3->ie=0;			/* interrupt enabled */

	dmaparams3->read_prio=0;/* read priority */
	dmaparams3->write_prio=0;/* write priority */

	dmaparams3->burst_mode=OMAP_DMA_DATA_BURST_DIS; /* Burst mode 4/8/16 words */

  
	printk(KERN_INFO "Doing DMA_request\n"); 
	/* chainable: */
	status = omap_request_dma(deviceRequestlineForDmaChannelsync, // The DMA request line to use; e.g. "OMAP24XX_DMA_MCBSP1_TX" for McBSP1 of (also) OMAP3530
				"McBSP RX test DMA for buffer 1 !",
				my_mcbsp_rx_dma_buf_callback,
				buffer1kernel,
				&buf1_dmachannel);
	printk(KERN_INFO "Done DMA_request!\n"); 
	if (status)
	{
		printk(KERN_ALERT  " Unable to request DMA channel for McBSP%d RX.\n",1);
		return -EAGAIN;
	}
	printk(KERN_ALERT  "Requested McBSP%d RX DMA channel %d\n",1, buf1_dmachannel);

	status = omap_request_dma(deviceRequestlineForDmaChannelsync, // The DMA request line to use; e.g. "OMAP24XX_DMA_MCBSP1_TX" for McBSP1 of (also) OMAP3530
				"McBSP RX test DMA for buffer 2 !",
				my_mcbsp_rx_dma_buf_callback,
				buffer2kernel,
				&buf2_dmachannel);
	if (status)
	{
		printk(KERN_ALERT " Unable to request DMA channel for McBSP%d RX.\n",1);
		return -EAGAIN;
	}
	printk(KERN_ALERT  "Requested McBSP%d RX DMA channel %d\n",1, buf2_dmachannel);

	status = omap_request_dma(deviceRequestlineForDmaChannelsync, // The DMA request line to use; e.g. "OMAP24XX_DMA_MCBSP1_TX" for McBSP1 of (also) OMAP3530
				"McBSP RX test DMA for buffer 3 !",
				my_mcbsp_rx_dma_buf_callback,
				buffer3kernel,
				&buf3_dmachannel);
	if (status)
	{
		printk(KERN_ALERT  " Unable to request DMA channel for McBSP%d RX.\n",1);
		return -EAGAIN;
	}
	printk(KERN_ALERT  "Requested McBSP%d RX DMA channel %d\n",1, buf3_dmachannel);

	// initialise the inter-thread waiting funtionality:



	/* Set up each DMA channel with it's parameters! */
	omap_set_dma_params(buf1_dmachannel, dmaparams1);
	omap_set_dma_params(buf2_dmachannel, dmaparams2);
	omap_set_dma_params(buf3_dmachannel, dmaparams3);


	/* Linking: Loop! */
	/* Set up this DMA channel to be linked to itself, thereby forming a one-buffer loop.
	 * Production should use at least two buffers so that one can be filled while the other is
	 * being read from. I just deactivated this (see e.g. simon_omap_mcbsp_recv_buffers() ) for testing. */
	//omap_dma_link_lch(buf1_dmachannel, buf1_dmachannel);

	/* Linking! Cycle through the 3 buffers, making a ring DMA transfer! See screen-output.txt! */
	omap_dma_link_lch(buf1_dmachannel, buf2_dmachannel);
	omap_dma_link_lch(buf2_dmachannel, buf3_dmachannel);
	omap_dma_link_lch(buf3_dmachannel, buf1_dmachannel);


	/* Begin the DMA transfers! The first 128 bytes may be wrong as the fifo buffer within the mcbsp port first has to be emptied. */
	omap_start_dma(buf1_dmachannel);

	/* Now wait until the DMA callback function has been called MAXCYCLES times. */
//	wait_for_completion(&mcbsp->rx_dma_completion);
	printk(KERN_ALERT "Not waiting for transfers to end. Continuing init function now!\n");



	__raw_writel(0xF4FFFF,ioremap( mcbsp_base_reg+8,4));  //start FPGA!!!
	printk(KERN_ALERT "Started FPGA\n");
	return 0;
}






static void my_mcbsp_rx_dma_buf_callback(int lch, u16 ch_status, void *data)
{
	u32 *bufferkernel;
	static bool overflow_err =0; //used to prevent log spamming...
	//int oldmm;
	if (data == NULL)
	{
		printk(KERN_ALERT " Skipping callback because data is NULL. Check initialisation order? \n");
		return;
	}

	bufferkernel = (u32*)data;

	if (bufferkernel == NULL)
	{
		printk(KERN_ALERT " Skipping callback because bufferkernel is NULL. Check initialisation order? \n");
		return;
	}
	if (!*bufferkernel)
	{
		printk(KERN_ALERT " Skipping callback because *bufferkernel is NULL. Check initialisation order? \n");
		return;
	}

	if(buffer_wait1_ptr == NULL)
	{
	  buffer_wait1_ptr = data;
	  overflow_err =0;
	}
	else if(buffer_wait1_ptr != NULL && buffer_wait2_ptr == NULL)
	{
	  buffer_wait2_ptr = data;
	  overflow_err =0;
	}
	else
	{
	  if(overflow_err==0)
	  {
	    printk(KERN_ALERT "Bufferoverflow\n");
	    overflow_err =1;
	  }
	}
}



u32 save_DMA_read(bool* r_err)
{
 static u32 element_count =0;
 u32 temp_read =0;


 while(buffer_read_ptr == NULL)
 {
  if(buffer_wait1_ptr != NULL ||buffer_wait2_ptr != NULL)
  {
    buffer_read_ptr = buffer_wait1_ptr;
    buffer_wait1_ptr = buffer_wait2_ptr;
    buffer_wait2_ptr = NULL;
  }
    //element_count =0;
 }

 temp_read = buffer_read_ptr[element_count];
 element_count++;
 if(element_count>= DMASIZE)
 {
   element_count =0;
   buffer_read_ptr =NULL;
 }
 return temp_read;
 
}

/*
u32 save_read(bool* r_err)
{
  u32 temp;
  if(0b100&__raw_readl(ioremap( mcbsp_base_reg+0x14,4)))
    *r_err |= true;
  while(!(0b10&__raw_readl(ioremap( mcbsp_base_reg+0x14,4))))  // 0x14 ersetzen durch spcr1-referenz  test for buffer empty
  {
    //printk(KERN_ALERT "EMPTY_err\n");
    //schedule_timeout(1);  
    
  }
  if(0b100&__raw_readl(ioremap( mcbsp_base_reg+0x14,4))) //do twice to prevent erros in case of interruption via process-shwitcher....
    *r_err |= true;
  
  temp= __raw_readl(ioremap( mcbsp_base_reg,4));
  return temp;//&0xff|((0b11100000&temp)<<3)|((0b11110&temp)<<10);
}
*/