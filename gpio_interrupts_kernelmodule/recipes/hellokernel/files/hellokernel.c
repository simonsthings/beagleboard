#include <linux/init.h>
#include <linux/module.h>
#include <linux/gpio.h>

#include <linux/sched.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>	/* We want an interrupt */
#include <asm/io.h>


MODULE_LICENSE("Dual BSD/GPL");

static int irq_gpio7 = -5;
static int irq_gpio139 = -7;
static int irq_gpio50 = -6;
static int togglevalue = 1;

irqreturn_t irq_handler(int irq, void *dev_id /*, struct pt_regs *regs*/)
{
	/* 
	 * This variables are static because they need to be
	 * accessible (through pointers) to the bottom half routine.
	 */
	//static int initialised = 0;
	//static unsigned char scancode;
	//static struct work_struct task;
	//unsigned char status;
	//int value = -3;

	// toggle gpio 138 (above 139) :
	gpio_direction_output(138,togglevalue);
	if (togglevalue == 0) 
	{
		togglevalue = 1;
	}
	else
	{
		togglevalue = 0;
	}

	/* 
	 * Read user button status
	 */
	//value = gpio_get_value(7);
	//printk(KERN_ALERT "The value of gpio 7 is currently: %d\n",value);
	//printk(KERN_ALERT "The ISR was fired! :-) \n");
	//printk(KERN_ALERT "The ISR was fired! Outputting %d on pin 5 (gpio138)...\n",togglevalue);

	return IRQ_HANDLED;
}


 int hello_init(void)
{
	int status = 0;
	int reqstatus = -4;
	int value = 0;
	int returnstatus = 3;

	printk(KERN_ALERT "Hello, world with gpio include!\n");
	status = gpio_is_valid(139);
	printk(KERN_ALERT "Is gpio 139 valid? -> %d\n",status);
	status = gpio_is_valid(7);
	printk(KERN_ALERT "Is gpio 7 valid? -> %d\n",status);
	status = gpio_is_valid(-4);
	printk(KERN_ALERT "Is gpio -4 valid? -> %d\n",status);

	reqstatus = gpio_request(7, "theuserbutton");
	printk(KERN_ALERT "Gpio 7 was requested and the answer is: %d\n",reqstatus);
	reqstatus = gpio_request(139, "mygpio139");
	printk(KERN_ALERT "Gpio 139 was requested and the answer is: %d\n",reqstatus);

	value = gpio_get_value(139);
	printk(KERN_ALERT "The value of gpio 139 is currently: %d\n",value);
	value = gpio_get_value(7);
	printk(KERN_ALERT "The value of gpio 7 is currently: %d\n",value);

	status = gpio_direction_input(139);
	printk(KERN_ALERT "Setting gpio139 as input -> %d\n",status);

	value = gpio_get_value(139);
	printk(KERN_ALERT "The value of gpio 139 is currently: %d\n",value);


	irq_gpio7 = gpio_to_irq(7);
	printk(KERN_ALERT "Gpio 7 is mapped to IRQ %d .\n",irq_gpio7);
	irq_gpio139 = gpio_to_irq(139);
	printk(KERN_ALERT "Gpio 139 is mapped to IRQ %d .\n",irq_gpio139);
	irq_gpio50 = gpio_to_irq(50);
	printk(KERN_ALERT "Gpio 50 is mapped to IRQ %d .\n",irq_gpio50);

	free_irq(irq_gpio7, NULL);
	free_irq(irq_gpio139, NULL);

	enable_irq(irq_gpio139);

	returnstatus = request_irq(irq_gpio139,	/* The number of the keyboard IRQ on PCs */
			   irq_handler,	/* our handler */
			   IRQF_SHARED | IRQF_TRIGGER_RISING, "test_gpio139_irq_handler",
			   (void *)(irq_handler));
	return returnstatus;
}

 void hello_exit(void)
{
	disable_irq(irq_gpio139);

	free_irq(irq_gpio7, NULL);
	free_irq(irq_gpio139, NULL);
	free_irq(irq_gpio50, NULL);
	gpio_free(50);
	gpio_free(139);
	gpio_free(7);
	printk(KERN_ALERT "Goodbye, cruel world\n");
}

module_init(hello_init);
module_exit(hello_exit);
