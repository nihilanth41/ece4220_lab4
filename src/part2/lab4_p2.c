#ifndef MODULE 
#define MODULE
#endif

#ifndef __KERNEL__
#define __KERNEL__
#endif

#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/time.h>
#include <unistd.h>
#include <rtai.h>
#include <rtai_sched.h>

MODULE_LICENSE("GPL");

#define NUM_PERIODS 1000 
#define FIFO_IN 0
RTIME period;
RT_TASK t1;
unsigned long *BasePtr, *PBDR, *PBDDR;	// pointers for port B DR/DDR

void rt_process(void *);

void rt_process(void *args) {
	// Open named pipe (get fd) -- write only
	// Opens for writing will block until a process opens it for reading
	// Unless specify O_NONBLOCK or O_NDELAY
	int fd = open("/dev/rtf/1", O_WRONLY);
	if(-1 == fd) 
	{
		// Error open
		return;
	}
	while(1)
	{
		//http://www.cs.uml.edu/~fredm/courses/91.308/files/pipes.html
		struct timeval tv;
		// Detect button press on PORT B0
		int button_status = (*PBDR & (1 << 0));
		if(0 == button_status)
		{
			// Button pressed -> get time
			 do_gettimeofday(&tv);
			// Attempt to write timeval struct to fifo
			ssize_t count = write(fd, &tv, sizeof(tv));
		}
		rt_wait_period();
	}
	close(fd);
}

int init_module(void) {
	// Attempt to map file descriptor
	BasePtr = (unsigned long *) __ioremap(0x80840000, 4096, 0);
	if(NULL == BasePtr) 
	{
		printk(KERN_INFO "Unable to map memory space\n");
		return -1;
	}

	// Run system() for now -- also,
	// man 2 mkfifo or man 2 mknod

	
	// Configure PORTB registers
	PBDR = BasePtr + 1;
	PBDDR = BasePtr + 5;
        // Set push button as input
	// button is PORTB0
	*PBDDR &= ~(1 << 0);
	
	// Start realtime timer
	rt_set_periodic_mode();
	// Task should 'go off' every 75ms
	// 1 ms = 1000000 ns
	// 75 ms = 75000000
	period = start_rt_timer(nano2count(75000000));

	// Initialize rt task
	rt_task_init(&t1, (void *)rt_process, 0, 256, 0, 0, 0);
	rt_task_resume(&t1);

	printk(KERN_INFO "MODULE INSTALLED\n");
	return 0;
}

void cleanup_module(void)
{
	rt_task_delete(&t1);
	stop_rt_timer();
	
	printk(KERN_INFO "MODULE REMOVED\n");
	return;
}
