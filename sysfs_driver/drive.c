#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/jiffies.h>

char *msg = "Bharath\n";
char ker_buf[PAGE_SIZE];

static DEFINE_MUTEX(locker);

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

	if (len > PAGE_SIZE)
		ret = -EINVAL;

	ret = simple_write_to_buffer(ker_buf,PAGE_SIZE,ppose,data,len);

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

/*This is read*/
static ssize_t id_show(struct kobject *kobj, struct kobj_attribute *attr , char *buf)
{
	return sprintf(buf, "%s\n",msg);
}

/*This is write*/
static ssize_t id_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	if (count > PAGE_SIZE)
		return -ENOMEM;

	if (strcmp(msg,buf) == 0)
		printk(KERN_ALERT "Nice!\n");
	else
		printk(KERN_ALERT "Sorry!\n");

	return count;
}

static struct kobj_attribute id_attribute = 
	__ATTR(id, 0664, id_show,id_store );

/*This is read*/
static ssize_t foo_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	mutex_lock(&locker);
	strncpy(buf,ker_buf,strlen(ker_buf));
	mutex_unlock(&locker);

	return strlen(ker_buf);
}

/*This is write*/
static ssize_t foo_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	if (count > PAGE_SIZE)
		return -EINVAL;

	mutex_lock(&locker);
	strncpy(ker_buf,buf,count);
	printk(KERN_DEBUG "The string is %s\n",ker_buf);
	mutex_unlock(&locker);

	return count;

}

static struct kobj_attribute foo_attribute = 
	__ATTR(foo, 0664, foo_show, foo_store);

static ssize_t jiff_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "%ld", jiffies);
}

static ssize_t jiff_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	return 0;
}

static struct kobj_attribute jiff_attribute = 
	__ATTR(jiff, 0444, jiff_show,jiff_store);

static struct attribute *attrs[] = {
	&id_attribute.attr,
	&foo_attribute.attr,
	&jiff_attribute.attr,
	NULL,
};

static struct attribute_group attr_group = {
	.attrs = attrs,
};

/* Module init and exit code */

static struct kobject *kobj_drive;

static int driver_mod_init(void)
{
	int retval;

	kobj_drive = kobject_create_and_add("kobj_drive", kernel_kobj);
	if (!kobj_drive)
		return -ENOMEM;
	
	retval = sysfs_create_group(kobj_drive, &attr_group);
	printk(KERN_ALERT "The ret val is %d\n",retval);
	if (retval == -EINVAL) {
		printk(KERN_ALERT "In retval\n");
	} else {
		printk(KERN_ALERT "It's something else\n");
	}

//	kobject_put(kobj_drive);
//	misc_register(&my_char);

	return 0;
}

void driver_mod_exit(void){

	kobject_put(kobj_drive);
//	misc_deregister(&my_char);
}

module_init(driver_mod_init);
module_exit(driver_mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Bharath Vedartham");
MODULE_DESCRIPTION("Misc char device driver");
MODULE_VERSION("0.01");


