#include<linux/init.h>
#include<linux/module.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/slab.h>
#include<linux/uaccess.h>
#include<linux/semaphore.h>
#include<linux/ioctl.h>
#include<linux/proc_fs.h>
#include<linux/jiffies.h>
#include<linux/sched.h>
#include<linux/time.h>
#include<linux/timer.h>
#include<linux/interrupt.h>
#include<asm/hardirq.h>

#define IOC_MAGIC 'k'
#define SCULL_IOC _IO(IOC_MAGIC,0)
#define SCULL_WRITE_IOC _IOW(IOC_MAGIC,1,char *)
#define SCULL_READ_IOC _IOR(IOC_MAGIC,2,char *)

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Abhishek");
MODULE_DESCRIPTION("Character driver");

#ifndef PRINTK
#define PRINTK
#endif

#ifndef QUANTUM
#define QUANTUM 8
#endif

#ifndef QSET
#define QSET 8
#endif

#ifndef DEVICE_SIZE
#define DEVICE_SIZE QUANTUM*QSET
#endif

#ifndef DATA_SIZE
#define DATA_SIZE 0
#endif

#ifndef DEVICE_NAME
#define DEVICE_NAME "Abhi_Device"
#endif

#ifndef NO_OF_DEV
#define NO_OF_DEV 1
#endif

#ifndef SCULL_MAJOR 
#define SCULL_MAJOR 0
#endif

#ifndef SCULL_MINOR
#define SCULL_MINOR 0
#endif

#ifndef DELAY_IN_JIFFIES
#define DELAY_IN_JIFFIES 1000
#endif
dev_t dev;

int scull_major = SCULL_MAJOR;
int scull_minor = SCULL_MINOR;
int scull_quantum = QUANTUM;
int scull_qset = QSET;
int scull_device_size= DEVICE_SIZE;
int scull_data_size = DATA_SIZE;
char proc_buffer[]="This is proc file";
struct timeval tv;
struct timespec ts,uptime1,uptime2;
unsigned long delay_in_jiffies = DELAY_IN_JIFFIES;
struct timer_list scull_timer;


struct Scull_qset
{
	void **data;
	struct Scull_qset *next;
};

struct Scull_dev
{
	struct Scull_qset *scull_qset;
	int quantum;
	int qset;
	unsigned long data_size;
	unsigned long device_size;
	struct semaphore sem;
	struct cdev cdev;
};
struct Scull_dev *scull_dev;
void scull_setup(struct Scull_dev *,int);
int scull_open(struct inode *, struct file *);
int scull_release(struct inode *, struct file *);
ssize_t scull_read(struct file *, char __user *, size_t, loff_t *);
ssize_t scull_write(struct file *, const char __user *, size_t, loff_t *);
loff_t scull_llseek(struct file *, loff_t, int);
long scull_ioctl(struct file *, unsigned int,unsigned long);



void  scull_qset_allocate(struct Scull_dev*);
void qset_allocate(struct Scull_dev*);
void quantum_allocate(struct Scull_dev *,int);
void timer_function( unsigned long );

struct file_operations fops=
	{
	open:scull_open,
	release:scull_release,
	write:scull_write,
	read:scull_read,
	llseek:scull_llseek,
	unlocked_ioctl:scull_ioctl
	

	};
void scull_trim(struct Scull_dev*);
struct proc_dir_entry *scull_proc,*scull_currentime,*scull_jitbusy,*scull_jitsched,*scull_jitqueue,*scull_jitimer;
