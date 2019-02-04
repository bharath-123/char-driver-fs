#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/semaphore.h>
#include <asm/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/string.h>

#define BUF_LEN 100

char *msg = "Bharath\n";
char ker_buf[BUF_LEN];
int right_size = sizeof(right) / sizeof(char);
int wrong_size = sizeof(wrong) / sizeof(char);

static ssize_t
driver_read(struct file *file, char __user *data, size_t len,
		loff_t *ppose)
{
	ssize_t bytes = sizeof(msg) / sizeof(char);

	printk(KERN_INFO "Data is read\n");
	printk(KERN_INFO "Size of output is %ld\n",bytes);

	return simple_read_from_buffer(data,len,ppose,msg,bytes);
}

static ssize_t
driver_write(struct file *file, const char __user *data, size_t len,
		loff_t *ppose)
{
	int ret; 

	if (len > BUF_LEN)
		ret = -EINVAL;

	ret = simple_write_to_buffer(ker_buf,BUF_LEN,ppose,data,len);

	printk(KERN_INFO "Data is written\n");
	printk(KERN_INFO "Data is %s\n",ker_buf);

	if (strcmp(msg,ker_buf) == 0)
		printk(KERN_INFO "You got it right!\n");
	else
		printk(KERN_INFO "You got it wrong!:/\n");
	
	return ret;
		
}

/*Module file operations*/

struct file_operations driver_ops = {
	.owner = THIS_MODULE,
	.read = driver_read,
	.write = driver_write
};

/*misc device*/
static struct miscdevice my_char = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "bharath",
	.fops = &driver_ops
};


/* Module init and exit code */

static int driver_mod_init(void)
{
	misc_register(&my_char);
	return 0;
}

void driver_mod_exit(void){
	misc_deregister(&my_char);
}

module_init(driver_mod_init);
module_exit(driver_mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Bharath Vedartham");
MODULE_DESCRIPTION("Misc char device driver");
MODULE_VERSION("0.01");


