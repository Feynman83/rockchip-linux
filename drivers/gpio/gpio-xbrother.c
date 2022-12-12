#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/pinctrl/pinctrl.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/platform_device.h>


static struct of_device_id gpio_export_ids[] = {
	{ .compatible = "gpio-export" },
	{ /* sentinel */ }
};
static int __init of_gpio_export_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct device_node *cnp;
	u32 val;
	int nb = 0;
    struct class * xbrother_class;
    struct device *gpio_dev;
    int err=0;
    const char *sys_name = NULL;
    
    xbrother_class = class_create(THIS_MODULE, "xbrother");
    gpio_dev = device_create(xbrother_class, NULL, 0, NULL, "gpio");

    of_property_read_string(np, "sys_name", &sys_name);
    if(!sys_name) {
        err=sysfs_create_link(NULL, &gpio_dev->kobj, "xbrother");
    }else{
        err=sysfs_create_link(NULL, &gpio_dev->kobj, sys_name);
    }
    
    if(err){
        dev_info(&pdev->dev, "sysfs_create_link:%d\n", err);
        return err;
    }
	for_each_child_of_node(np, cnp) {
		const char *name = NULL;
		int gpio;
		bool dmc;
		int max_gpio = 1;
		int i;
		of_property_read_string(cnp, "gpio-export,name", &name);
		if (!name)
			max_gpio = of_gpio_count(cnp);
		for (i = 0; i < max_gpio; i++) {
			unsigned flags = 0;
			enum of_gpio_flags of_flags;
			gpio = of_get_gpio_flags(cnp, i, &of_flags);
			if(!gpio_is_valid(gpio))
				continue;
			if (of_flags == OF_GPIO_ACTIVE_LOW)
				flags |= GPIOF_ACTIVE_LOW;
			if (!of_property_read_u32(cnp, "gpio-export,output", &val))
				flags |= val ? GPIOF_OUT_INIT_HIGH : GPIOF_OUT_INIT_LOW;
			else
				flags |= GPIOF_IN;
			if (devm_gpio_request_one(&pdev->dev, gpio, flags, name ? name : of_node_full_name(np)))
				continue;
			dmc = of_property_read_bool(cnp, "gpio-export,direction_may_change");
            gpio_export(gpio, dmc);
            if(name){
                gpio_export_link(gpio_dev, name, gpio);
            }
			nb++;
		}
	}
	dev_info(&pdev->dev, "%d gpio(s) exported\n", nb);
	return 0;
}
static struct platform_driver gpio_export_driver = {
	.driver		= {
		.name		= "gpio-export",
		.owner	= THIS_MODULE,
		.of_match_table	= of_match_ptr(gpio_export_ids),
	},
};
static int __init of_gpio_export_init(void)
{
	return platform_driver_probe(&gpio_export_driver, of_gpio_export_probe);
}
late_initcall(of_gpio_export_init);