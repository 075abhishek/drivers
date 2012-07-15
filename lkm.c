#include"header.h"

void timer_function( unsigned long arg )
{
	int len;
	char *buffer = (char *)arg;
	unsigned long j = jiffies;
	len=0;
	#ifdef PRINTK
	printk(KERN_INFO"Called:%s\n at %li",__func__,jiffies);
	#endif

	len= sprintf(buffer,"%9li\t%i\t %6i    %i\n",j,in_interrupt()?1:0,current->pid,smp_processor_id());
	
}

int scull_proc_jitimer(char *buffer, char **location,off_t off,int buff_len,int *eof,void *dat)
{
	unsigned long j;
	int len,lv,data_written;
	len =lv=data_written=0;
	len = sprintf(buffer,"Jiffies      in_interrupt  PID    CPU\n");
	init_timer(&scull_timer);
	scull_timer.function = timer_function;
	for(lv=0;lv<5;lv++)
	{
	j=jiffies;
	scull_timer.expires = j+10;
	data_written = strlen(buffer);
	scull_timer.data = (unsigned long)(buffer + data_written);
	add_timer(&scull_timer);
	while(time_before(jiffies,j +11 ))
	schedule();
	}
	len =strlen(buffer);
	#ifdef PRINTK
	printk(KERN_INFO"len:%d\n",len);
	#endif
	return len;
}

int scull_proc_jitqueue(char *buffer, char **location,off_t off,int buff_len,int *eof,void *data)
{
	int len;
	unsigned long j,k;
	len =0;
	j=jiffies;
	len = sprintf(buffer,"%li \t",(long)j);
	set_current_state(TASK_INTERRUPTIBLE);
	schedule_timeout(delay_in_jiffies);
	k=jiffies;
	len += sprintf(buffer + len,"%li \n",(long)k);
	return len;
}
int scull_proc_jitsched(char *buffer, char **location,off_t off,int buff_len,int *eof,void *data)
{
	unsigned long j1;
	int len;
	len = 0;
	j1=jiffies;
	len = sprintf(buffer,"%li\t",j1);
	while(time_before(jiffies,j1 + delay_in_jiffies))
	schedule();
	len += sprintf(buffer + len,"%li\n",(long)jiffies);
	buffer = buffer + len;
	return len;
}

int scull_proc_jitbusy(char *buffer, char **location,off_t off,int buff_len,int *eof,void *data)
{
	unsigned long j1;
	int len;
	len = 0;
	j1=jiffies;
	len = sprintf(buffer,"%li\t",j1);
	while(time_before(jiffies,j1+ delay_in_jiffies))
	cpu_relax();
	len += sprintf(buffer + len,"%li\n",(long)jiffies);
	buffer = buffer + len;
	
	return len;
}

int scull_proc_currentime(char *buffer, char **location,off_t off,int buff_len,int *eof,void *data)
{
	int len;
	unsigned long jify;
	u64 jiff;
	len =0;
	jify=jiffies; 
	jiff= get_jiffies_64();
	do_gettimeofday(&tv);
	ts= current_kernel_time();
	len = sprintf(buffer ,"In the proc_currentime\n jiffies %li\njiffies_64 %lli\ntv_usec %li\ntv_nsec %li",jify,jiff,(long)tv.tv_usec,(long)ts.tv_nsec);
	
	return len;
}
int scull_proc_read(char *buffer, char **location,off_t off,int buff_len,int *eof,void *data)
{
	#ifdef PRINTK
	printk(KERN_INFO"Begin:%s\n",__func__);
	#endif
	memcpy(buffer,proc_buffer,sizeof(proc_buffer));
	#ifdef PRINTK
	printk(KERN_INFO"End :%s\n",__func__);
	#endif
	return sizeof(proc_buffer);
}

long scull_ioctl(struct file *filp, unsigned int cmd,unsigned long arg)
{
	char temp;
	char ch ='B';
	#ifdef PRINTK
	printk(KERN_INFO"Begin:%s\n",__func__);
	#endif
	switch(cmd)
	{
		case SCULL_IOC:	
				#ifdef PRINTK
				printk(KERN_INFO"In the ioctl command\n");
				#endif
				break;
		case SCULL_WRITE_IOC:	
				#ifdef PRINTK
				printk(KERN_INFO"In the ioctl write command\n");
				#endif
				get_user(temp,(char *)arg);
				#ifdef PRINTK
				printk(KERN_INFO"Message is %c\n",temp);
				#endif
		
				break;
		case SCULL_READ_IOC:	
				#ifdef PRINTK
				printk(KERN_INFO"In the ioctl read command\n");
				#endif
				put_user(ch,(char *)arg);
		
				break;
		default:
				#ifdef PRINTK
				printk(KERN_INFO"Wrong ioctl command\n");
				#endif
	}
	#ifdef PRINTK
	printk(KERN_INFO"End :%s\n",__func__);
	#endif
	return 0;
}


loff_t scull_llseek(struct file *filp, loff_t offset, int origin)
{
	struct Scull_dev *lscull_dev;
	#ifdef PRINTK
	printk(KERN_INFO"Begin:%s\n",__func__);
	#endif
	lscull_dev = filp->private_data;
	switch(origin)
	{
		case 0: filp->f_pos = offset;
			#ifdef PRINTK
			printk(KERN_INFO"Seeking  from begining\n");
			#endif
			break;
		case 1: filp->f_pos += offset;
			#ifdef PRINTK
			printk(KERN_INFO"Seeking  from current\n");
			#endif
			break;
		case 2:filp->f_pos = lscull_dev->data_size - offset;
			#ifdef PRINTK
			printk(KERN_INFO"Seeking  from End\n");
			#endif
			break;
	}
	
	
	#ifdef PRINTK
	printk(KERN_INFO"End :%s\n",__func__);
	#endif
	return filp->f_pos;
}

void quantum_allocate(struct Scull_dev *fscull_dev,int fquantum)
{
	int lv;
	#ifdef PRINTK
	printk(KERN_INFO"Begin:%s\n",__func__);
	#endif
	for(lv=0;lv<fquantum;lv++)
	{
		fscull_dev->scull_qset->data[lv] = kmalloc(scull_quantum,GFP_KERNEL);
		
		memset(fscull_dev->scull_qset->data[lv],'\0',scull_quantum);
		if(!fscull_dev->scull_qset->data[lv])
		{
			#ifdef PRINTK
			printk(KERN_ERR"End E:Quantum %d not allocated\n",lv);
			#endif
		}
		
		#ifdef PRINTK
		printk(KERN_INFO"Quantum %d \t%p\n",lv,fscull_dev->scull_qset->data[lv]);
		#endif
	}
	#ifdef PRINTK
	printk(KERN_INFO"End :%s\n",__func__);
	#endif
}
void qset_allocate(struct Scull_dev * fscull_dev)
{
	#ifdef PRINTK
	printk(KERN_INFO"Begin:%s\n",__func__);
	#endif
	fscull_dev->scull_qset->data = kmalloc(sizeof(char*) * (scull_qset),GFP_KERNEL);
	memset(fscull_dev->scull_qset->data ,'\0',(sizeof(char*) * (scull_qset)));
		if(!fscull_dev->scull_qset->data)
		{
			#ifdef PRINTK
			printk(KERN_ERR"End E:Qset not allocated\n");
			#endif
		
		}
	

	#ifdef PRINTK
	printk(KERN_INFO"End :%s\n",__func__);
	#endif
}
void scull_qset_allocate(struct Scull_dev* fscull_dev)
{
	#ifdef PRINTK
	printk(KERN_INFO"Begin:%s\n",__func__);
	#endif
	fscull_dev->scull_qset = kmalloc(sizeof(struct Scull_qset),GFP_KERNEL);
	
	memset(fscull_dev->scull_qset,'\0',(sizeof(struct Scull_qset)));
	fscull_dev->scull_qset->data = NULL;
	fscull_dev->scull_qset->next = NULL;
	#ifdef PRINTK
	printk(KERN_INFO"End :%s\n",__func__);
	#endif

}

ssize_t scull_write(struct file *filp, const char __user *ubuff, size_t size, loff_t *loff)
{
	int nctw,ncsw,noq,lv,retval;
	struct Scull_dev *lscull_dev;

	ncsw= nctw =noq=retval=0;
	#ifdef PRINTK
	printk(KERN_INFO"Begin:%s\n",__func__);
	printk(KERN_INFO"Position at write starrt %d :\n",(int)filp->f_pos);
	#endif
	lscull_dev=filp->private_data;
	if(!lscull_dev)
	{
		#ifdef PRINTK
		printk(KERN_ERR"No Device in private->data\n");
		#endif
	}

	if(size > lscull_dev->device_size)
	{
		#ifdef PRINTK
		printk(KERN_ERR"Data is too big.Partial write\n");
		#endif
	}

	
	if(lscull_dev->data_size==0)
	{
		scull_qset_allocate(lscull_dev);
		if(!lscull_dev->scull_qset)
		{
			#ifdef PRINTK
			printk(KERN_ERR"Scull_qset not allocated\n");
			#endif
		}
		qset_allocate(lscull_dev);

	noq = size / scull_quantum;
	if(size%scull_quantum !=0)
		noq++;

	quantum_allocate(lscull_dev,noq);
	for(lv=0;lv<noq;lv++)
	{
		if(!lscull_dev->scull_qset->data[lv])
		{
			#ifdef PRINTK
			printk(KERN_ERR"End E:Quantum %d not allocated\n",lv);
			#endif
		}
	
		#ifdef PRINTK
		printk(KERN_INFO"Quantum %d \t%p\n",lv,lscull_dev->scull_qset->data[lv]);
		#endif
	}
	for(lv=0;lv<noq;lv++)
	{
		if((size-ncsw) >scull_quantum)
			nctw=scull_quantum;
		else
			nctw=size-ncsw;
		retval=	copy_from_user(lscull_dev->scull_qset->data[lv],ubuff + ncsw ,nctw);
		ncsw = ncsw + nctw - retval;
		#ifdef PRINTK
		printk(KERN_INFO"Quantum %d \t%s\n",lv,(char *)lscull_dev->scull_qset->data[lv]);
		#endif
	}
	lscull_dev->data_size = ncsw;
	}

	filp->f_pos= filp->f_pos +ncsw;


	#ifdef PRINTK
	printk(KERN_INFO"Position after write end %d :\n",(int)filp->f_pos);
	#endif
	#ifdef PRINTK
	printk(KERN_INFO"End :%s\n",__func__);
	#endif

	return ncsw;
}

ssize_t scull_read(struct file *filp,char __user *ubuff, size_t size, loff_t *loff)
{
	int ncsr,nctr,cq,noq,current_size,retval,q_pos;
	struct Scull_dev *lscull_dev;
	
	#ifdef PRINTK
	printk(KERN_INFO"Begin:%s\n",__func__);
	printk(KERN_INFO"Position at read start %d :\n",(int)filp->f_pos);
	#endif
	lscull_dev = filp->private_data;
	retval=ncsr = nctr = cq = noq = current_size=q_pos=0;
	
	if(!lscull_dev)
	{
		#ifdef PRINTK
		printk(KERN_ERR"No Device in private->data\n");
		#endif
	}
	if(lscull_dev->data_size == 0)
	{
		#ifdef PRINTK
		printk(KERN_ERR"No Data to read\n");
		goto END;
		#endif
	}
	q_pos = (int)filp->f_pos / scull_quantum;
	cq = (int)filp->f_pos % scull_quantum;
	if(size > lscull_dev->data_size)
	current_size = lscull_dev->data_size - filp->f_pos;

	else
	current_size = size - filp->f_pos;


	while(lscull_dev->scull_qset->data[q_pos])
	{
		if((current_size - ncsr) > scull_quantum)
		nctr = scull_quantum - cq;
		else
		nctr = current_size -ncsr-cq;
		
		retval = copy_to_user(ubuff + ncsr,lscull_dev->scull_qset->data[q_pos]+cq,nctr);
		ncsr = ncsr + nctr - retval;
		
		#ifdef PRINTK
		printk(KERN_INFO"Quantum %d \t%s\n",q_pos,(char *)lscull_dev->scull_qset->data[q_pos]+cq);
		#endif
		q_pos++;
		cq =0;
	}


		
	lscull_dev->data_size = 0;
	filp->f_pos = filp->f_pos + ncsr; 
	#ifdef PRINTK
	printk(KERN_INFO"Position after read end %d :\n",(int)filp->f_pos);
	printk(KERN_INFO"End :%s\n",__func__);
	#endif
	return ncsr;
END:
	
	return 0;
}

void scull_trim(struct Scull_dev *lscull_dev)
{
	struct Scull_dev *fscull_dev;
	int lv=0;
	#ifdef PRINTK
	printk(KERN_INFO"Begin:%s\n",__func__);
	#endif

	fscull_dev = lscull_dev;
	if(!fscull_dev)
	{
		#ifdef PRINTK
		printk(KERN_ERR"No Device\n");
		#endif
	}
	if(!fscull_dev->scull_qset)
	
	{
		#ifdef PRINTK
		printk(KERN_ERR"No scullQset\n");
		#endif
	}
	else
	{
		for(lv=7;lv>0;lv--)
		{
		 if((fscull_dev->scull_qset->data[lv]) != NULL)
		   {
			kfree(fscull_dev->scull_qset->data[lv]);
		   }
		}
		kfree(fscull_dev->scull_qset->data);
		kfree(fscull_dev->scull_qset);
		fscull_dev->scull_qset=NULL;
		fscull_dev->data_size = 0;
		#ifdef PRINTK
		printk(KERN_INFO"Scull trimmed.\n");
		#endif
	}
	
	#ifdef PRINTK
	printk(KERN_INFO"End :%s\n",__func__);
	#endif
	
}
int scull_open(struct inode *inodp, struct file *filp)
{
	struct Scull_dev *lscull_dev;
	#ifdef PRINTK
	printk(KERN_INFO"Begin:%s\n",__func__);
	#endif
	lscull_dev = container_of(inodp->i_cdev,struct Scull_dev,cdev);
	if(!lscull_dev)
		{	
		#ifdef PRINTK
		printk(KERN_ERR"End E:Error in container_of\n");
		#endif

		}
	filp->private_data = lscull_dev;
//check for access mode
	if((filp->f_flags & O_ACCMODE)==O_RDONLY)
	{
		#ifdef PRINTK
		printk(KERN_INFO"File opened in read only mode\n");
		#endif
	}
	
//trim if in write only mode
	if((filp->f_flags & O_ACCMODE)==O_WRONLY)
	{
		#ifdef PRINTK
		printk(KERN_INFO"File opened in write only mode\n");
		#endif
		
		scull_trim(scull_dev);
	
	}


	filp->private_data = lscull_dev;
	#ifdef PRINTK
	printk(KERN_INFO"End:%s\n",__func__);
	#endif
	return 0;
}

int scull_release(struct inode *inodp, struct file *filp)
{
	#ifdef PRINTK
	printk(KERN_INFO"Begin:%s\n",__func__);
	#endif

	#ifdef PRINTK
	printk(KERN_INFO"End:%s\n",__func__);
	#endif
	return 0;
}

void scull_setup(struct Scull_dev *lscull_dev,int nod)
{
	cdev_init(&scull_dev->cdev,&fops );
	scull_dev-> cdev.owner= THIS_MODULE;
	scull_dev->cdev.ops = &fops;
	cdev_add(&scull_dev->cdev,dev,1);	
}

static int __init initilisation_function(void)
{
	int retval;
//	unsigned long j1,hz,diff;
	#ifdef PRINTK
	printk(KERN_INFO"Begin:%s\n",__func__);
	#endif

/*	diff = 1000;
	j1=jiffies;
	do_posix_clock_monotonic_gettime(&uptime1);
	monotonic_to_bootbased(&uptime1);
	while(time_before(jiffies,j1 + diff))
	schedule();
	do_posix_clock_monotonic_gettime(&uptime2);
	monotonic_to_bootbased(&uptime2);
	hz = diff /((uptime2.tv_sec)-(uptime1.tv_sec));
	#ifdef PRINTK
	printk(KERN_INFO"Value of Hz:%li\n",hz);
	#endif

*/	
	scull_proc = create_proc_entry("Scull_proc",0,NULL);
	scull_proc->read_proc = scull_proc_read;
	
	scull_currentime = create_proc_entry("Scull_currentime",0,NULL);
	scull_currentime->read_proc = scull_proc_currentime;
	
	scull_jitbusy = create_proc_entry("Scull_jitbusy",0,NULL);
	scull_jitbusy->read_proc = scull_proc_jitbusy;
	
	scull_jitsched = create_proc_entry("Scull_jitsched",0,NULL);
	scull_jitsched->read_proc = scull_proc_jitsched;
	
	scull_jitqueue = create_proc_entry("Scull_jitqueue",0,NULL);
	scull_jitqueue->read_proc = scull_proc_jitqueue;

	scull_jitimer = create_proc_entry("Scull_jitimer",0,NULL);
	scull_jitimer->read_proc = scull_proc_jitimer;
//////
//////
	#ifdef PRINTK
	printk(KERN_INFO"Proc files created\n");
	#endif
//////
	if(scull_major)
	{

	dev = MKDEV(scull_major,scull_minor+1);
	retval = register_chrdev_region(dev,NO_OF_DEV,DEVICE_NAME);	

	if(retval < 0)
	{
	
		#ifdef PRINTK
		printk(KERN_ERR"Register_chardev_region failed\n");
		#endif
		goto END;
	}
	printk(KERN_INFO"Major No:%d\tMinor No:%d\n",scull_major,scull_minor);
	}
	else
	{
	retval= alloc_chrdev_region(&dev, 0, NO_OF_DEV,DEVICE_NAME);
	if(retval < 0)
	{
		
		#ifdef PRINTK
		printk(KERN_ERR"Alloc_chardev_region failed\n");
		#endif
		goto END;
	}
	}

	scull_major =MAJOR(dev);
	scull_minor =MINOR(dev);
	
	#ifdef PRINTK
	printk(KERN_INFO"Major No:%d\tMinor No:%d\n",scull_major,scull_minor);
	#endif

//Allocation of memory
	scull_dev=kmalloc(sizeof(struct Scull_dev)*NO_OF_DEV,GFP_KERNEL);
	if(!scull_dev)
	{
		#ifdef PRINTK
		printk(KERN_ERR"Kmalloc Error\n");
		#endif
		goto END;
	}

//memset
	memset(scull_dev,'\0',(NO_OF_DEV)*sizeof(struct Scull_dev));	
//initialisation
	
	scull_dev->qset= scull_qset;
	scull_dev->quantum=scull_quantum;
	scull_dev->device_size=scull_device_size;
	scull_dev->data_size=scull_data_size;

	#ifdef PRINTK
	printk(KERN_INFO"Qset:%d\n",scull_dev->qset);
	printk(KERN_INFO"Quantum:%d\n",scull_dev->quantum);
	printk(KERN_INFO"Device size:%ld\n",scull_dev->device_size);
	printk(KERN_INFO"Data size:%ld\n",scull_dev->data_size);
	#endif
//semaphore
//	sema_init(&scull_dev->sem,1);
//	
//Scull setup
	scull_setup(scull_dev,NO_OF_DEV);
	
	#ifdef PRINTK
	printk(KERN_INFO"End:%s\n",__func__);
	#endif
	return 0;
END:
	#ifdef PRINTK
	printk(KERN_INFO"End E:%s\n",__func__);
	#endif
	return -1;
}

static void __exit cleanup_function(void)
{
	#ifdef PRINTK
	printk(KERN_INFO"Begin:%s\n",__func__);
	#endif
	unregister_chrdev_region(dev,NO_OF_DEV);
	cdev_del(&scull_dev->cdev);
	del_timer_sync(&scull_timer);
	remove_proc_entry("Scull_proc",NULL);
	remove_proc_entry("Scull_currentime",NULL);
	remove_proc_entry("Scull_jitbusy",NULL);
	remove_proc_entry("Scull_jitsched",NULL);
	remove_proc_entry("Scull_jitqueue",NULL);
	remove_proc_entry("Scull_jitimer",NULL);
	#ifdef PRINTK
	printk(KERN_INFO"End:%s\n",__func__);
	#endif

}
module_init(initilisation_function);
module_exit(cleanup_function);
