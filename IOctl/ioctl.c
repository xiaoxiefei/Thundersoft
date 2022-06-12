#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/fs.h>
#include <linux/fcntl.h>
#include <linux/ctype.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/pagemap.h>
#include <linux/errno.h>
#include <linux/types.h>

#define BUF_SIZE 256

#define IO_MAJOR 255
#define IO_MINOR 0

#define COMMAND_A 'a'
#define COMMAND_B 'b'

struct io_dev{
	struct cdev cdev;
};

static struct io_dev * io_devices;
static struct class *io_class;
static unsigned char io_count = 0;
static u8 io_buf[BUF_SIZE];

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
	printk(KERN_ERR "Line %d, function %s() has been invoked!\n", __LINE__, __func__);
	switch(cmd){
		case COMMAND_A:
			printk("DEMO:Line %d, ioctl successflly (COMMAND_A)!\n", __LINE__);
			break;
		case COMMAND_B:
			printk("DEMO:Line %d, ioctl successfully (COMMAND_B)!\n", __LINE__);
			break;
		default:
			printk("DEMO:Line %d, ioctl error (incalid command!\n", __LINE__);
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
	.llseek		= io_llseek,
	.read		= io_read,
	.write		= io_write,
	.unlocked_ioctl	= io_ioctl,
	.open		= io_open,
	.release	= io_release,
};

static int __init io_init(void){
	int retval;
	dev_t devno = 0;
	printk(KERN_ERR "Line %d, function %s() has been invoked!\n", __LINE__, __func__);
	devno = MKDEV(IO_MAJOR, IO_MINOR);
	printk("devno = %d\n", devno);
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

static void __exit io_exit(void){
	dev_t devno = MKDEV(IO_MAJOR, IO_MINOR);

	device_destroy(io_class, devno);
	class_destroy(io_class);
	printk(KERN_ERR "Line %d, function %s() has been invoked!\n", __LINE__, __func__);
	if(io_devices){
		cdev_del(&io_devices->cdev);
		kfree(io_devices);
	}
	unregister_chrdev_region(devno, 1);
}

module_init(io_init);
module_exit(io_exit);
MODULE_AUTHOR("EDU");
MODULE_LICENSE("GPL");

