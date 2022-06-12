#include <linux/kernel.h>
#include <asm/signal.h>
#include <linux/param.h>
#include <asm/processor.h>
#include <linux/completion.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/version.h>
#include <linux/fs.h>
#include <linux/fcntl.h>
#include <linux/ctype.h>
#include <linux/pagemap.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/semaphore.h>
#include <linux/init.h>
#include <linux/kdev_t.h>
#include <linux/device.h>

#define BUF_SIZE 256

#define IO_MAJOR 255
#define IO_MINOR 0

//线程1
#define COMMAND_a 'a' //开始
#define COMMAND_b 'b' //暂停
#define COMMAND_c 'c' //继续
#define COMMAND_d 'd' //重新开始
#define COMMAND_e 'e' //停止

//线程2
#define COMMAND_A 'A'
#define COMMAND_B 'B'
#define COMMAND_C 'C'
#define COMMAND_D 'D'
#define COMMAND_E 'E'

//所有线程
#define COMMAND_1 '1'
#define COMMAND_2 '2'
#define COMMAND_3 '3'
#define COMMAND_4 '4'
#define COMMAND_0 '0'


//控制线程的信号
static struct semaphore sem_1;
static struct semaphore sem_2;

//字符设备
struct io_dev{
	struct cdev cdev;
};

static struct io_dev * io_devices;
static struct class *io_class;
static unsigned char io_count = 0;
static u8 io_buf[BUF_SIZE];

//线程
static struct task_struct *task1;
static struct task_struct *task2;

int flag1 = 0, flag2 = 0;

static loff_t io_llseek(struct file *filp, loff_t off, int whence){
	loff_t pos = filp->f_pos;
	printk(KERN_ERR "Line %d, function %s() has been invoked!\n", __LINE__, __func__);
	switch(whence){
		case 0:
			pos = off;
			break;
		case 1:
			pos += off;
			break;
		case 2:
		default:
			return -EFAULT;
	}
	if((pos > BUF_SIZE) || (pos < 0)){
		return -EINVAL;
	}
	return filp->f_pos = pos;
}

static ssize_t io_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos){
	ssize_t retval = 0;
	loff_t pos = *f_pos;
	printk(KERN_ERR "Line %d, function %s() has been invoked!\n", __LINE__, __func__);
	if(pos > BUF_SIZE){
		printk(KERN_ERR "Line %d, pos is overflow!\n", __LINE__);
		count = -EINVAL;
		goto out;
	}
	if(count > (BUF_SIZE -pos)){
		count = BUF_SIZE -pos;
	}
	pos += count;
	if(copy_to_user(buf, io_buf + *f_pos, count)){
		count = -EFAULT;
		goto out;
	}
	*f_pos = pos;
	retval = count;
out:
	return retval;
}

static ssize_t io_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos){
	ssize_t retval = 0;
	loff_t pos = *f_pos;
	printk(KERN_ERR "Line %d, function %s() has been invoked!\n", __LINE__, __func__);
	if(pos > BUF_SIZE){
		printk(KERN_ERR "Line %d, pos is overflow!\n", __LINE__);
		count = -EINVAL;
		goto out;
	}
	if(count > (BUF_SIZE - pos)){
		count = BUF_SIZE - pos;
	}
	pos += count;
	if(copy_from_user(io_buf + *f_pos, buf, count)){
		count = -EFAULT;
		goto out;
	}
	*f_pos = pos;
	retval = count;
out:
	return retval;
}

static long io_ioctl(struct file *filp, unsigned int cmd, unsigned long arg){
	//int retval = 0;
	printk(KERN_ERR "Line %d, function %s() has been invoked!\n", __LINE__, __func__);
	if(sem_1.count == 5 || sem_2.count == 5){
		if((cmd >= 'b') && (cmd <= 'e')){
			printk("The thread-1 has not started yet!\n");
			return 0;
		}
		else if((cmd >= 'B') && (cmd <= 'E')){
			printk("The thread-2 has not started yet!\n");
			return 0;
		}
		else if(((cmd >= '2') && (cmd <= '4')) || (cmd == '0')){
			if(sem_1.count == 5){
				printk("The thread-1 has not started yet!\n");
			}
			if(sem_2.count == 5){
				printk("The thread-2 has not started yet!\n");
			}
			return 0;
		}
	}
	
	switch(cmd){
		//控制第一个线程
		case COMMAND_a:
			//retval = down_interruptible(&sem);
			sem_1.count = 1;
			printk("DRIVER:Line %d, Thread-1 start successflly sem = %d !\n", __LINE__, sem_1.count);
			break;
		case COMMAND_b:
			//up(&sem);
			sem_1.count = 2;
			printk("DRIVER:Line %d, Thread-1 pause successflly sem = %d !\n", __LINE__, sem_1.count);
			break;
		case COMMAND_c:
			sem_1.count = 3;
			printk("DRIVER:Line %d, Thread-1 continue successflly sem = %d !\n", __LINE__, sem_1.count);
			break;
		case COMMAND_d:
			sem_1.count = 4;
			printk("DRIVER:Line %d, Thread-1 restart successflly sem = %d !\n", __LINE__, sem_1.count);
			break;
		case COMMAND_e:
			sem_1.count = 0;
			printk("DRIVER:Line %d, Thread-1 stop successflly sem = %d !\n", __LINE__, sem_1.count);
			break;
		
		//控制第二个线程
		case COMMAND_A:
			sem_2.count = 1;
			printk("DRIVER:Line %d, Thread-2 start successflly sem = %d !\n", __LINE__, sem_2.count);
			break;
		case COMMAND_B:
			sem_2.count = 2;
			printk("DRIVER:Line %d, Thread-2 pause successflly sem = %d !\n", __LINE__, sem_2.count);
			break;
		case COMMAND_C:
			sem_2.count = 3;
			printk("DRIVER:Line %d, Thread-2 continue successflly sem = %d !\n", __LINE__, sem_2.count);
			break;
		case COMMAND_D:
			sem_2.count = 4;
			printk("DRIVER:Line %d, Thread-2 restart successflly sem = %d !\n", __LINE__, sem_2.count);
			break;
		case COMMAND_E:
			sem_2.count = 0;
			printk("DRIVER:Line %d, Thread-2 stop successflly sem = %d !\n", __LINE__, sem_2.count);
			break;

		//控制所有线程
		case COMMAND_1:
			sem_1.count = 1;
			sem_2.count = 1;
			printk("DRIVER:Line %d, All-Thread start successflly sem = %d !\n", __LINE__, sem_1.count);
			break;
		case COMMAND_2:
			sem_1.count = 2;
			sem_2.count = 2;
			printk("DRIVER:Line %d, All-Thread pause successflly sem = %d !\n", __LINE__, sem_1.count);
			break;
		case COMMAND_3:
			sem_1.count = 3;
			sem_2.count = 3;
			printk("DRIVER:Line %d, All-Thread continue successflly sem = %d !\n", __LINE__, sem_1.count);
			break;
		case COMMAND_4:
			sem_1.count = 4;
			sem_2.count = 4;
			printk("DRIVER:Line %d, All-Thread restart successflly sem = %d !\n", __LINE__, sem_1.count);
			break;
		case COMMAND_0:
			sem_1.count = 0;
			sem_2.count = 0;
			printk("DRIVER:Line %d, All-Thread stop successflly sem = %d !\n", __LINE__, sem_1.count);
			break;
		default:
			printk("DRIVER:Line %d, ioctl error (invaild command)!\n", __LINE__);
			return -EINVAL;
	}
	return 0;
}

static int io_open(struct inode *inode, struct file *filp){
	struct io_dev *dev;
	printk(KERN_ERR "Line %d, function %s() has been invoked!\n", __LINE__, __func__);
	if(io_count > 0){
		return -EINVAL;
	}
	io_count++;
	dev = container_of(inode->i_cdev, struct io_dev, cdev);
	filp->private_data = dev;
	return 0;
}

static int io_release(struct inode *inode, struct file *filp){
	printk(KERN_ERR "Line %d, function %s() has been invoked!\n", __LINE__, __func__);
	io_count--;
	return 0;
}

static struct file_operations io_fops = {
	.owner		= THIS_MODULE,
	.llseek         = io_llseek,
	.read           = io_read,
	.write          = io_write,
	.unlocked_ioctl	= io_ioctl,
	.open           = io_open,
	.release        = io_release,
};

static int thread1_work(void *data){
	int i = 1;
	printk("New kernel thread-1 run\n");
	//allow_signal(SIGKILL);
	mdelay(1000);
	sema_init(&sem_1, 5);
//	printk("sem_1.count = %d\n", sem_1.count);
	while((!kthread_should_stop())&&sem_1.count){ 
		set_current_state(TASK_INTERRUPTIBLE);
		switch(sem_1.count){
			case 1:
				printk("Thread-1 : %d\n", (i++)%101);
				schedule_timeout(HZ * 1);
				break;
			case 2:
				schedule_timeout(HZ * 0.1);
				break;
			case 3:
				printk("Thread-1 : %d\n", (i++)%101);
				schedule_timeout(HZ * 1);
				break;
			case 4:
				i = 1;
				sem_1.count = 1;
				printk("Thread-1 : %d\n", (i++)%101);
				schedule_timeout(HZ * 1);
				break;
			default:
				schedule_timeout(HZ * 0.1);
		}
		//printk("should stop : %d\n", kthread_should_stop());
	}
	printk("thread1_work finished\n");
	flag1 = 1;
	return 0;
}

static int thread2_work(void *data){
	int j = 1;
	printk("              New kernel thread-2 run\n");
	mdelay(1000);
	while(!kthread_should_stop()&&sem_2.count){
		set_current_state(TASK_INTERRUPTIBLE);
		switch(sem_2.count){
			case 1:
				printk("              Thread-2 : %d\n", (j++)%101);
				schedule_timeout(HZ * 2);
				break;
			case 2:
				schedule_timeout(HZ * 0.1);
				break;
			case 3:
				printk("              Thread-2 : %d\n", (j++)%101);
				schedule_timeout(HZ * 2);
				break;
			case 4:
				j = 1;
				sem_2.count = 1;
				printk("              Thread-2 : %d\n", (j++)%101);
				schedule_timeout(HZ * 2);
				break;
			default:
				schedule_timeout(HZ * 0.1);
		}
	}
	printk("              thread2_work finished!\n");
	flag2 = 1;
	return 0;
}

static int __init test_init(void)
{
	/* Schedule the test thread */
	//kthread_run (thread_work, NULL, "thread_1");

	int retval;
	dev_t devno = 0;
	printk(KERN_ERR "Line %d, function %s() has been invoked!\n", __LINE__, __func__);
	devno = MKDEV(IO_MAJOR, IO_MINOR);
	printk(KERN_ERR "Line %d, major = %d, minor = %d!\n", __LINE__, MAJOR(devno), MINOR(devno));
	retval = register_chrdev_region(devno, 1, "io_test");
	if(retval){
		printk(KERN_ERR "Line %d, function %s() has been invoked!\n", __LINE__, __func__);
		goto out;
	}
	io_devices = kmalloc(sizeof(struct io_dev), GFP_KERNEL);
	if(!io_devices){
		retval = -ENOMEM;
		goto error_cdev;
	}
	memset(io_devices, 0, sizeof(struct io_dev));
	cdev_init(&io_devices->cdev, &io_fops);
	io_devices->cdev.owner = THIS_MODULE;
	//io_devices->cdev.owner = drive;
	io_devices->cdev.ops = &io_fops;
	retval = cdev_add(&io_devices->cdev, devno, 1);
	if(retval){
		printk(KERN_ERR "Line %d, failed to add char_dev io_test!\n", __LINE__);
		goto error_cdev;
	}

	//让udev自动创建和删除设备文件
	io_class = class_create(THIS_MODULE, "aston_class");
	if(IS_ERR(io_class)){
		return -EINVAL;
	}
	device_create(io_class, NULL, devno, NULL, "command");


	task1 = kthread_create(thread1_work, NULL, "task1");
	wake_up_process(task1);
	task2 = kthread_create(thread2_work, NULL, "task2");
	wake_up_process(task2);
	printk("init wait for completion()\n");

	sema_init(&sem_1, 5);
	sema_init(&sem_2, 5);

	return 0;

out:
	return retval;

error_cdev:
	if(io_devices){
		cdev_del(&io_devices->cdev);
		kfree(io_devices);
	}
	unregister_chrdev_region(devno, 1);
	return retval;
}
 
static void __exit test_exit(void)
{
	int ret ;

	dev_t devno = MKDEV(IO_MAJOR, IO_MINOR);
	device_destroy(io_class, devno);
	class_destroy(io_class);
	printk(KERN_ERR "Line %d, function %s() has been invoked!\n", __LINE__, __func__);
	if(io_devices){
		cdev_del(&io_devices->cdev);
		kfree(io_devices);
	}
	unregister_chrdev_region(devno, 1);

	if(!flag1){
		if(!IS_ERR(task1)){
			ret = kthread_stop(task1);
			printk(KERN_INFO "First thread function has stoped return %d\n",ret);
		}
	}
	printk("Task1 stop success\n");

	if(!flag2){
		if(!IS_ERR(task2)){
			ret = kthread_stop(task2);
			printk(KERN_INFO "Second thread function has stoped return %d\n", ret);
		}
	}
	printk("Task2 stop success\n");

	return ;
}
 
MODULE_LICENSE("GPL");
module_init(test_init);
module_exit(test_exit);


