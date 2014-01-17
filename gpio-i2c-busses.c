#include <linux/device.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/i2c-gpio.h>
#include <linux/gpio.h>
#include <plat/sys_config.h>
#include <linux/slab.h>
#include <linux/string.h>

struct  gpio_i2c_desc {
	struct list_head list;
	struct i2c_gpio_platform_data *pdata;
	struct platform_device *pdevice;
};

struct platform_device_info plat_dev_info = {
	.name = "i2c-gpio",
	.id = -2,
	.data = NULL,
	.size_data = sizeof(struct i2c_gpio_platform_data),
};

static int i2c_dev_cnt;

static DEFINE_MUTEX(sysfs_lock);

LIST_HEAD(i2c_gpio_list);


static ssize_t export_store(struct class *class, struct class_attribute *attr,
				const char *buf, size_t len)
{

	int sda_pin, scl_pin, id;

	struct gpio_i2c_desc *i2c_dev_item = NULL;
	struct i2c_gpio_platform_data *pdata = NULL;

	sscanf(buf, "%d,%d,%d", &sda_pin, &scl_pin, &id);
	mutex_lock(&sysfs_lock);

	/* check if gpio is valid */
	if (!gpio_is_valid(sda_pin))
		goto invalid_gpio;

	if (!gpio_is_valid(scl_pin))
		goto invalid_gpio;

	i2c_dev_item = kzalloc(sizeof(struct gpio_i2c_desc), GFP_KERNEL);
	if (i2c_dev_item == NULL)
		goto i2c_dev_item_alloc_err;

	/* now try to allocate platform data */
	pdata = kzalloc(sizeof(struct i2c_gpio_platform_data), GFP_KERNEL);
	if (pdata == NULL)
		goto pdata_alloc_err;

	/* fill the platform data with sda and scl pin */
	pdata->sda_pin = sda_pin;
	pdata->scl_pin = scl_pin;

	/* fill the platform driver info  with id and platform data */
	plat_dev_info.id = id;
	plat_dev_info.data = pdata;

	i2c_dev_item->pdata = pdata;

	/* register the new gpio i2c master bus */
	i2c_dev_item->pdevice  = platform_device_register_full(&plat_dev_info);

	if (i2c_dev_item->pdevice != NULL) {
		INIT_LIST_HEAD(&i2c_dev_item->list);
		list_add(&i2c_dev_item->list, &i2c_gpio_list);
		i2c_dev_cnt++;
		mutex_unlock(&sysfs_lock);
		return len;
	}
	kfree(pdata);

pdata_alloc_err:
	kfree(i2c_dev_item);

i2c_dev_item_alloc_err:
invalid_gpio:

	mutex_unlock(&sysfs_lock);
	return -EINVAL;
}

static ssize_t unexport_store(struct class *class,
			struct class_attribute *attr,
			const char *buf, size_t len)
{
	int id;
	struct gpio_i2c_desc *ptr, *next;
	/* get id interface we want to remove */
	mutex_lock(&sysfs_lock);
	sscanf(buf, "%d", &id);

	list_for_each_entry_safe(ptr, next, &i2c_gpio_list, list) {
	if (ptr->pdevice->id == id) {
		platform_device_unregister(ptr->pdevice);
		kfree(ptr->pdata);
		kfree(ptr->pdevice);
		list_del(&ptr->list);
		kfree(ptr);
		mutex_unlock(&sysfs_lock);
		return len;
		}
	}

	mutex_unlock(&sysfs_lock);
	return -EINVAL;
}

static struct class_attribute gpio_i2c_busses_class_attrs[] = {
	__ATTR(export,   0200, NULL, export_store),
	__ATTR(unexport, 0200, NULL, unexport_store),
	__ATTR_NULL,
};


void gpio_i2c_busses_class_release(struct class *class)
{
}

static struct class gpio_i2c_busses_class = {
	.name = "gpio_i2c_busses",
	.owner = THIS_MODULE,
	.class_attrs = gpio_i2c_busses_class_attrs,
	.class_release = gpio_i2c_busses_class_release,
};


static int __init i2c_gpio_busses_init(void)
{
	int status;
	i2c_dev_cnt = 0;
	status = class_register(&gpio_i2c_busses_class);
	printk(KERN_INFO "I2C gpio busses class initialized.\n");
	return status;
}

static void __exit i2c_gpio_busses_exit(void)
{
struct gpio_i2c_desc *ptr, *next;
/* free all gpio_i2c_desc */
list_for_each_entry_safe(ptr, next, &i2c_gpio_list, list) {
	platform_device_unregister(ptr->pdevice);
	kfree(ptr->pdata);
	kfree(ptr->pdevice);
	list_del(&ptr->list);
	kfree(ptr);
}

class_unregister(&gpio_i2c_busses_class);
printk(KERN_INFO "I2C gpio busses class clean up\n");
}

module_init(i2c_gpio_busses_init);
module_exit(i2c_gpio_busses_exit);

MODULE_DESCRIPTION("I2C GPIO busses class");
MODULE_AUTHOR("Philippe Van Hecke <lemouchon@gmail.com>");
MODULE_LICENSE("GPL");
