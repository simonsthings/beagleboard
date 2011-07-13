#define KERNEL32

#include <config/modversions.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/completion.h>
#include <linux/dma-mapping.h>
#include <linux/gpio.h>
#include <plat/gpio.h>
#include <plat/dma.h>

#define OMAP34XX_GPIO1_BASE             IO_ADDRESS(0x48310000)
#define OMAP34XX_GPIO2_BASE             IO_ADDRESS(0x49050000)
#define OMAP34XX_GPIO3_BASE             IO_ADDRESS(0x49052000)
#define OMAP34XX_GPIO4_BASE             IO_ADDRESS(0x49054000)
#define OMAP34XX_GPIO5_BASE             IO_ADDRESS(0x49056000)
#define OMAP34XX_GPIO6_BASE             IO_ADDRESS(0x49058000)

#define OMAP34XX_GPIO_SYSCONFIG         0x0010
#define OMAP34XX_GPIO_SYSSTATUS         0x0014
#define OMAP34XX_GPIO_IRQSTATUS1        0x0018
#define OMAP34XX_GPIO_IRQSTATUS2        0x0028
#define OMAP34XX_GPIO_IRQENABLE2        0x002c
#define OMAP34XX_GPIO_IRQENABLE1        0x001c
#define OMAP34XX_GPIO_WAKE_EN           0x0020
#define OMAP34XX_GPIO_CTRL              0x0030
#define OMAP34XX_GPIO_OE                0x0034
#define OMAP34XX_GPIO_DATAIN            0x0038
#define OMAP34XX_GPIO_DATAOUT           0x003c
#define OMAP34XX_GPIO_LEVELDETECT0      0x0040
#define OMAP34XX_GPIO_LEVELDETECT1      0x0044
#define OMAP34XX_GPIO_RISINGDETECT      0x0048
#define OMAP34XX_GPIO_FALLINGDETECT     0x004c
#define OMAP34XX_GPIO_DEBOUNCE_EN       0x0050
#define OMAP34XX_GPIO_DEBOUNCE_VAL      0x0054
#define OMAP34XX_GPIO_CLEARIRQENABLE1   0x0060
#define OMAP34XX_GPIO_SETIRQENABLE1     0x0064
#define OMAP34XX_GPIO_CLEARWKUENA       0x0080
#define OMAP34XX_GPIO_SETWKUENA         0x0084
#define OMAP34XX_GPIO_CLEARDATAOUT      0x0090
#define OMAP34XX_GPIO_SETDATAOUT        0x0094

#define MAXCYCLES 50
int cyclecounter = 0;
u32 fullbuf[64*MAXCYCLES];

dma_addr_t buffer1;
u32* buff_array1;

dma_addr_t buffer2;
u32* buff_array2;


void omap_gpio_write(void __iomem *io_base, u16 reg, u32 val)    
{
	if (cpu_class_is_omap1() || cpu_is_omap2420())
		__raw_writew((u16)val, io_base + reg);
	else
		__raw_writel(val, io_base + reg);
}

int omap_gpio_read(void __iomem *io_base, u16 reg)
{
	if (cpu_class_is_omap1() || cpu_is_omap2420())
		return __raw_readw(io_base + reg);
	else
		return __raw_readl(io_base + reg);
}


static void omap_gpio_dma_buf_callback(int lch, u16 ch_status, void *data)
{
	char printtemp[500];
	int i;
	int bufbufsize = 64; // number of array elements
	u32* bufbuf2 = *((u32**)data);
	
	//output something:
	printk(KERN_ALERT "Using callback function \n");

       // for (i = 0 ; i<bufbufsize; i++)
	//{
	//	fullbuf[i + cyclecounter * bufbufsize] = bufbuf2[i];
	//}
        //cyclecounter++;

	//if (cyclecounter >= MAXCYCLES)

        //{	
	/* We can free the channels */
	//omap_free_dma(lch);
    

	printk(KERN_ALERT "The values of the transferbuffer bufbuf are: \n",bufbufsize);
	for (i = 0 ; i<bufbufsize; i++)
	{
		sprintf(printtemp, "%s 0x%x,", printtemp,bufbuf2[i]);

		if ((i%10) == 0)
		{
			printk(KERN_ALERT "%s \n",printtemp);
			sprintf(printtemp, "   ");
		}
	}
	printk(KERN_ALERT " end. \n");

    for (i = 0 ; i<64; i++)
	{
	printk(KERN_ALERT "The buff_array2[%d] is: 0x%x \n",i, buff_array2[i]);
	}
        
        //}
}


/*************************************************************************************************************************/
/*************************************************Set up DMA**************************************************************/
/*************************************************************************************************************************/
 int omap_gpio_recv_buffers(dma_addr_t buffer1,u32* buff_array1, unsigned int length)
 {
    char printtemp[500];
    int src_port = 0;
    int dest_port = 0;
	int sync_dev = 0;
	int status = 3; // dummy value    
	int deviceRequestlineForDmaChannelsync = 0;
        struct omap_dma_channel_params *dmaparams;
	int buf_dmachannel;
        int i;
	//dma_addr_t buffer2;
        //u32* buff_array2;

     //   buff_array2 = dma_alloc_coherent(NULL, 256 , &buffer2, GFP_KERNEL);
     //   if (buff_array2 == NULL) {pr_err("Unable to allocate DMA buffer 2\n");return -ENOMEM;}

	dmaparams = kzalloc(sizeof(struct omap_dma_channel_params), GFP_KERNEL);   // allocate memory for dma parameter.
	if (!dmaparams) {
		return -ENOMEM;
	}
	
	deviceRequestlineForDmaChannelsync = OMAP24XX_DMA_MCBSP2_TX  ; // Use external DMA channel 3
	
	dmaparams->data_type=OMAP_DMA_DATA_TYPE_S32;		/* data type 8,16,32 */
	dmaparams->elem_count=64;		/* number of elements in a frame */
	dmaparams->frame_count=1;	/* number of frames in a element */
	
	dmaparams->src_port=0;		/* Only on OMAP1 REVISIT: Is this needed? */
	dmaparams->src_amode=OMAP_DMA_AMODE_POST_INC;		/* constant, post increment, indexed,double indexed */  
	dmaparams->src_start=buffer1;		/* source address : physical */
	dmaparams->src_ei=0;		/* source element index */
	dmaparams->src_fi=0;		/* source frame index */
 
        dmaparams->dst_port=0;		/* Only on OMAP1 REVISIT: Is this needed? */
	dmaparams->dst_amode = OMAP_DMA_AMODE_POST_INC;		/* constant, post increment, indexed,double indexed */   
	dmaparams->dst_start = buffer2;		/* destination address : physical */
	dmaparams->dst_ei=0;		/* source element index */
	dmaparams->dst_fi=0;		/* source frame index */
	
	dmaparams->trigger=deviceRequestlineForDmaChannelsync;		/* trigger attached if the channel is synchronized */
	dmaparams->sync_mode=OMAP_DMA_SYNC_ELEMENT;		/* sycn on element, frame , block or packet */
	dmaparams->src_or_dst_synch=0;	/* source synch(1) or destination synch(0) */
	
	dmaparams->ie=0;			/* interrupt enabled */

	dmaparams->read_prio=0;/* read priority */
	dmaparams->write_prio=0;/* write priority */

	dmaparams->burst_mode=OMAP_DMA_DATA_BURST_DIS; /* Burst mode 4/8/16 words */

    printk(KERN_ALERT "The values in buffer 1 before DMA transfer : \n");
    for (i = 0 ; i<64; i++)
	{
         printk(KERN_ALERT "The buff_array1[%d] is: 0x%x \n",i, buff_array1[i]);
	}

    printk(KERN_ALERT "The values in buffer 2 before DMA transfer : \n");
    for (i = 0 ; i<64; i++)
	{
	printk(KERN_ALERT "The buff_array2[%d] is: 0x%x \n",i, buff_array2[i]);
	}

    printk(KERN_ALERT "the physic address of dma channel is : 0x%lx \n",deviceRequestlineForDmaChannelsync );
	
    status = omap_request_dma(deviceRequestlineForDmaChannelsync,                //// see page 978 for external DMA request
    			      " GPIO test DMA for buffer  !",
    			        omap_gpio_dma_buf_callback,
    				&buff_array2,
    				&buf_dmachannel);
				
    if (status)
	{
		printk(KERN_ALERT " Unable to request DMA channel for GPIO\n");
		return -EAGAIN;
	}	
        
    else   printk (KERN_ALERT "DMA channel for GPIO is ready to use \n");
   
    //init_completion();  //// initialise the inter-thread waiting funtionality:

    sync_dev = OMAP24XX_DMA_MCBSP2_TX;

   	/* Set up each DMA channel with it's parameters! */
	omap_set_dma_params(buf_dmachannel, dmaparams);

	
    //omap_dma_link_lch(buf_dmachannel, buf_dmachannel);	

    omap_start_dma(buf_dmachannel);

   //wait_for_completion ();
         
    printk(KERN_ALERT "The values in buffer 2 after DMA transfer : \n");
    for (i = 0 ; i<64; i++)
	{
	printk(KERN_ALERT "The buff_array2[%d] is: 0x%x \n",i, buff_array2[i]);
	}

    return 0;
		
  //omap_free_dma(buf_dmachannel);		
 }


static int __init hello_init(void)
{

 int i;
 u32* buff_array;
 int returnstatus = 0; 
 int status = 3;
 
 /* Number of elements (values) in each DMA buffer.*/
 int bufbufsize = 64; // number of array elements
 /* Number of bytes needed to store each value: */
 int bytesPerVal = 4; // number of bytes per array element (32bit = 4 bytes, 16bit = 2 bytes)
 
 dma_addr_t bufbufdmaaddr;   /* The pointers to the same DMA buffers for use only by DMA controller: */
 
 /* Allocate memory space for the three buffers and assign the 6 pointers: */
 buff_array = dma_alloc_coherent(NULL, bufbufsize*bytesPerVal, &bufbufdmaaddr, GFP_KERNEL);
 if (buff_array == NULL) {pr_err("Unable to allocate DMA buffer 1\n");return -ENOMEM;}

 buff_array2 = dma_alloc_coherent(NULL, 256 , &buffer2, GFP_KERNEL);
 if (buff_array2 == NULL) {pr_err("Unable to allocate DMA buffer 2\n");return -ENOMEM;}

 

 /***********************************************************************************************************/
 /* Write 64 values to buff_array, these values will be later written into gpio 5 and gpio 6 output register*/
 /***********************************************************************************************************/
  
 for (i=0; i<bufbufsize; i++)
 {
  buff_array[i] = 0xbf0A0000 | (i+1);  
  printk(KERN_ALERT "The buff_array[%d] is: %x \n",i, buff_array[i]);
 }
 	
 /* DMA */
 /* Set up, start, end, and diplay the dma transfer: */
 
 printk(KERN_ALERT "Now reading data from buffer via DMA! \n");
 status = omap_gpio_recv_buffers(bufbufdmaaddr,buff_array, 1); // the dma memory must have been allocated correctly.
 printk(KERN_ALERT "Read from GPIO via DMA! Return status: %d \n", status);
 
 /* Display the contents of the GPIO output after the transfer: */

 
 
 printk(KERN_ALERT " end. \n");
 
 return returnstatus;
}


static void __exit hello_exit(void)
{
 printk (KERN_ALERT "Goodbye GPIO  \n");
}

module_init(hello_init);
module_exit(hello_exit);