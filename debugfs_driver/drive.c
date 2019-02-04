#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/mutex.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/debugfs.h>
#include <linux/jiffies.h>

#define BUF_LEN PAGE_SIZE

static DEFINE_MUTEX(locker);

//mutex_init(locker);

char *msg = "Bharath\n";
char ker_buf[BUF_LEN];
struct dentry *bhar_dir, *id, *jiff_file, *foo;
int file_val = 5;
u64 ran_val;

static ssize_t
driver_read(struct file *file, char __user *data, size_t len,
		loff_t *ppose)
{
	ssize_t bytes = sizeof(msg) / sizeof(char);

	printk(KERN_INFO "Data is read\n");
	printk(KERN_INFO "Size of output is %ld\n",bytes);

	return simple_read_from_buffer(data,len,ppose,msg,8);
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

static ssize_t
debug_read(struct file *file, char __user *data,size_t len,
		loff_t *ppose)
{
	ssize_t bytes = sizeof(ker_buf) / sizeof(char);
	int ret;

	if(len > BUF_LEN)
		return -EINVAL;

	printk(KERN_DEBUG "In debug_read\n");
	mutex_lock(&locker);
	ret = simple_read_from_buffer(data,len,ppose,ker_buf,bytes);
	mutex_unlock(&locker);

	return ret;
}

static ssize_t
debug_write(struct file *file, const char __user *data,size_t len,
		loff_t *ppose)
{
	int ret; 

	printk(KERN_DEBUG "In debug_write\n");	
	if (len > BUF_LEN)
		return -EINVAL;

	mutex_lock(&locker);
	ret = simple_write_to_buffer(ker_buf,BUF_LEN,ppose,data,len);
	mutex_unlock(&locker);

	return ret;
	
}

/*Module file operations*/

struct file_operations driver_ops = {
	.owner = THIS_MODULE,
	.read = driver_read,
	.write = driver_write
};

struct file_operations debug_ops = {
	.owner = THIS_MODULE,
	.read = debug_read,
	.write = debug_write
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
	/*Create bharath dir in debugfs*/
	bhar_dir = debugfs_create_dir("bharath",NULL);
	if (!bhar_dir)
		goto fail;
	
	/*Create id,jiff_file and foo file*/
	id = debugfs_create_file("id", 0777, bhar_dir, NULL, &driver_ops);
	if (!id)
		goto fail;

	jiff_file = debugfs_create_u64("jiff_file",0444,bhar_dir,(u64*)&jiffies);
	if(!jiff_file)
		goto fail;

	foo = debugfs_create_file("foo",0644,bhar_dir,NULL,&debug_ops);
	if (!foo)
		goto fail;

	misc_register(&my_char);
	return 0;

fail: pr_alert("Could not create devices\n");
      return -ENODEV;
}

void driver_mod_exit(void){
	debugfs_remove_recursive(bhar_dir);
	misc_deregister(&my_char);
}

module_init(driver_mod_init);
module_exit(driver_mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Bharath Vedartham");
MODULE_DESCRIPTION("Misc char device driver");
MODULE_VERSION("0.01");


