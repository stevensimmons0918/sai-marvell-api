/*******************************************************************************
Copyright (C) Marvell International Ltd. and its affiliates

This software file (the "File") is owned and distributed by Marvell
International Ltd. and/or its affiliates ("Marvell") under the following
alternative licensing terms.  Once you have made an election to distribute the
File under one of the following license alternatives, please (i) delete this
introductory statement regarding license alternatives, (ii) delete the two
license alternatives that you have not elected to use and (iii) preserve the
Marvell copyright notice above.

********************************************************************************
Marvell Commercial License Option

If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.

********************************************************************************
Marvell GPL License Option

If you received this File from Marvell, you may opt to use, redistribute and/or
modify this File in accordance with the terms and conditions of the General
Public License Version 2, June 1991 (the "GPL License"), a copy of which is
available along with the File in the license.txt file or by writing to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 or
on the worldwide web at http://www.gnu.org/licenses/gpl.txt.

THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE IMPLIED
WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY
DISCLAIMED.  The GPL License provides additional details about this warranty
disclaimer.
********************************************************************************
*/
/**
********************************************************************************
* @file prestera.c
*
* @brief functions in kernel mode special for prestera.
*
*/

//#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/pci.h>
#include <linux/msi.h>
#include <linux/gpio/driver.h>
#include <linux/gpio/consumer.h>
#include <linux/workqueue.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(5, 0, 0)
#include <linux/gpio/machine.h>
#endif

#define MODULE_NAME             "mvGpioDrv"
#define GPIO_DRV_VER			"1.02"

#define PCI_DEVICE_ID_MRVL_AC3X	0xc804
#define GPIO_NUM_0_OFFSET       0x00018100
#define GPIO_NUM_1_OFFSET       0x00018140
#define GPIO_NPINS              64

/* ET6448M Board specific */
#define PSU_LED_GREEN_PIN       21
#define PSU_LED_AMBER_PIN       22
#define FAN_LED_GREEN_PIN       26
#define FAN_LED_AMBER_PIN       25
#define PSU_LED_GREEN(x)        (x + PSU_LED_GREEN_PIN)
#define PSU_LED_AMBER(x)        (x + PSU_LED_AMBER_PIN)
#define FAN_LED_GREEN(x)        (x + FAN_LED_GREEN_PIN)
#define FAN_LED_AMBER(x)        (x + FAN_LED_AMBER_PIN)
#define GPIO_PIN_CHIPBASE(p)    (prv_data->chip0.base + p)

#define VERBOSE_DBG             0

struct gpio_desc_1 {
    struct gpio_device      *gdev;
    unsigned long           flags; 
    /* Connection label */         
    const char              *label;         
    /* Name of the GPIO */         
    const char              *name; 
};

typedef struct mv_gpio_reg
{
    uint32_t gpio_data_out;
    uint32_t gpio_data_out_ena;
    uint32_t gpio_data_blink;
    uint32_t gpio_data_polarity;
    uint32_t gpio_data_in;
    uint32_t gpio_intr_cause;
    uint32_t gpio_intr_mask;
    uint32_t gpio_intr_lvl_mask;
    uint32_t gpio_blink_cntr_sel;
    uint32_t gpio_ctrl_set;
    uint32_t gpio_ctrl_clr;
    uint32_t gpio_data_out_set;
    uint32_t gpio_data_out_clr;
    uint32_t gpio_blink_cntr_a_on_dur;
    uint32_t gpio_blink_cntr_a_off_dur;
    uint32_t gpio_blink_cntr_b_on_dur;
    uint32_t gpio_blink_cntr_b_off_dur;
} mv_gpio_reg_t;

typedef struct mv_gpio_prv
{
    struct device *dev;
	struct pci_dev *pdev;
    const struct pci_device_id * dev_id;
    const char * name;
    void __iomem * resource_0;
    mv_gpio_reg_t __iomem * reg0;
    mv_gpio_reg_t __iomem * reg1;
    struct gpio_chip chip0;
    int driver_init_state;
    int device_id;
    struct gpio_desc * gdesc[GPIO_NPINS];
    raw_spinlock_t lock;
} mv_gpio_prv_t;

static const char * mv_gpio0_pin_names[GPIO_NPINS] = {
    "d0p0", "d0p1", "d0p2", "d0p3", "d0p4", "d0p5", "d0p6", "d0p7", 
    "d0p8", "d0p9", "d0p10", "d0p11", "d0p12", "d0p13", "d0p14", "d0p15", 
    "d0p16", "d0p17", "d0p18", "d0p19", "d0p20", "psuLedAmber", "psuLedGreen", "d0p23", 
    "d0p24", "fanLedGreen", "fanLedAmber", "d0p27", "d0p28", "d0p29", "d0p30", "d0p31", 
    "d0p32", "d0p33", "d0p34", "d0p35", "d0p36", "d0p37", "d0p38", "d0p39", 
    "d0p40", "d0p41", "d0p42", "d0p43", "d0p44", "d0p45", "d0p46", "d0p47", 
    "d0p48", "d0p49", "d0p50", "d0p51", "d0p52", "d0p53", "d0p54", "d0p55", 
    "d0p56", "d0p57", "d0p58", "d0p59", "d0p60", "d0p61", "d0p62", "d0p63"
};

static const char * mv_gpio1_pin_names[GPIO_NPINS] = {
    "d1p0", "d1p1", "d1p2", "d1p3", "d1p4", "d1p5", "d1p6", "d1p7", 
    "d1p8", "d1p9", "d1p10", "d1p11", "d1p12", "d1p13", "d1p14", "d1p15", 
    "d1p16", "d1p17", "d1p18", "d1p19", "d1p20", "d1p21", "d1p22", "d1p23", 
    "d1p24", "d1p25", "d1p26", "d1p27", "d1p28", "d1p29", "d1p30", "d1p31", 
    "d1p32", "d1p33", "d1p34", "d1p35", "d1p36", "d1p37", "d1p38", "d1p39", 
    "d1p40", "d1p41", "d1p42", "d1p43", "d1p44", "d1p45", "d1p46", "d1p47", 
    "d1p48", "d1p49", "d1p50", "d1p51", "d1p52", "d1p53", "d1p54", "d1p55", 
    "d1p56", "d1p57", "d1p58", "d1p59", "d1p60", "d1p61", "d1p62", "d1p63"
};

static int driver_ref_count = 0;

static int mv_gpio_pcie_map(mv_gpio_prv_t *prv_data)
{
	prv_data->resource_0 = pcim_iomap(prv_data->pdev, 0, 1*1024*1024); /* CnM registers AC3X/BC2/Aldrin only*/
	if (!prv_data->resource_0)
		pr_err("%s: Cannot map!\n", __func__);
    prv_data->reg0 = (mv_gpio_reg_t *)((uint8_t *)prv_data->resource_0 + GPIO_NUM_0_OFFSET);
    prv_data->reg1 = (mv_gpio_reg_t *)((uint8_t *)prv_data->resource_0 + GPIO_NUM_1_OFFSET);

	return -1*(prv_data->resource_0 == NULL);
}

static int mv_gpio_pcie_unmap(mv_gpio_prv_t *prv_data)
{
	pcim_iounmap(prv_data->pdev, prv_data->resource_0);
	return 0;
}

static int mv_gpio_get_direction(struct gpio_chip *chip, unsigned int pin)
{
	int ret;
	uint32_t val32;
    mv_gpio_prv_t *prv_data = gpiochip_get_data(chip);

#if VERBOSE_DBG
    dev_info(prv_data->dev, "%#x:%#x gpio_data_out_0 %#x gpio_data_out_1 %#x\n", 
             prv_data->dev_id->vendor, prv_data->dev_id->device, prv_data->reg0->gpio_data_out, prv_data->reg1->gpio_data_out);
    dev_info(prv_data->dev, "%#x:%#x gpio_data_out_ena_0 %#x gpio_data_out_ena_1 %#x\n", 
             prv_data->dev_id->vendor, prv_data->dev_id->device, prv_data->reg0->gpio_data_out_ena, prv_data->reg1->gpio_data_out_ena);
#endif //VERBOSE_DBG

	ret = mv_gpio_pcie_map(prv_data);
	if (ret)
		return ret;
	
    if (pin < 32)
    {
        val32 = prv_data->reg0->gpio_data_out_ena & (1<<pin);
		mv_gpio_pcie_unmap(prv_data);
		return val32;
    }
    else if (pin < 64)
    {
        val32 = prv_data->reg1->gpio_data_out_ena & (1<<pin);
		mv_gpio_pcie_unmap(prv_data);
		return val32;
    }

	mv_gpio_pcie_unmap(prv_data);

    return 1;
}

static int mv_gpio_direction_input(struct gpio_chip *gpio, unsigned int pin)
{
	int ret;
    mv_gpio_prv_t *prv_data = gpiochip_get_data(gpio);

    raw_spin_lock(&prv_data->lock);

	ret = mv_gpio_pcie_map(prv_data);
	if (ret) {
		raw_spin_unlock(&prv_data->lock);
		return ret;
		}

    // Set 1 for Input dir
    if (pin < 32)
    {
        prv_data->reg0->gpio_data_out_ena |= (1<<pin);
    }
    else if (pin < 64)
    {
        pin -= 32;
        prv_data->reg1->gpio_data_out_ena |= (1<<pin);
    }

	mv_gpio_pcie_unmap(prv_data);

    raw_spin_unlock(&prv_data->lock);
#if VERBOSE_DBG
    dev_info(prv_data->dev, "%#x:%#x gpio_data_out_0 %#x gpio_data_out_1 %#x\n", 
             prv_data->dev_id->vendor, prv_data->dev_id->device, prv_data->reg0->gpio_data_out, prv_data->reg1->gpio_data_out);
    dev_info(prv_data->dev, "%#x:%#x gpio_data_out_ena_0 %#x gpio_data_out_ena_1 %#x\n", 
             prv_data->dev_id->vendor, prv_data->dev_id->device, prv_data->reg0->gpio_data_out_ena, prv_data->reg1->gpio_data_out_ena);
#endif //VERBOSE_DBG

    return 0;
}

static int mv_gpio_direction_output(struct gpio_chip *gpio, unsigned int pin, int val)
{
    int ret;
    mv_gpio_prv_t *prv_data = gpiochip_get_data(gpio);

    // Set val to out
    // Set 0 for Output dir
    raw_spin_lock(&prv_data->lock);
    ret = mv_gpio_pcie_map(prv_data);
    if (ret) {
        raw_spin_unlock(&prv_data->lock);
        return ret;
    }

    if (pin < 32)
    {
        if (val)
        {
            prv_data->reg0->gpio_data_out |= (1<<pin);
        }
        else
        {
            prv_data->reg0->gpio_data_out &= (~(1<<pin));
        }
        prv_data->reg0->gpio_data_out_ena &= (~(1<<pin));
    }
    else if (pin < 64)
    {
        pin -= 32;
        if (val)
        {
            prv_data->reg1->gpio_data_out |= (1<<pin);
        }
        else
        {
            prv_data->reg1->gpio_data_out &= (~(1<<pin));
        }
        prv_data->reg1->gpio_data_out_ena &= (~(1<<pin));
    }

    mv_gpio_pcie_unmap(prv_data);

    raw_spin_unlock(&prv_data->lock);
#if VERBOSE_DBG
    dev_info(prv_data->dev, "%#x:%#x gpio_data_out_0 %#x gpio_data_out_1 %#x\n", 
            prv_data->dev_id->vendor, prv_data->dev_id->device, prv_data->reg0->gpio_data_out, prv_data->reg1->gpio_data_out);
    dev_info(prv_data->dev, "%#x:%#x gpio_data_out_ena_0 %#x gpio_data_out_ena_1 %#x\n", 
            prv_data->dev_id->vendor, prv_data->dev_id->device, prv_data->reg0->gpio_data_out_ena, prv_data->reg1->gpio_data_out_ena);
#endif //VERBOSE_DBG

    return 0;
}

static int mv_gpio_get(struct gpio_chip *chip, unsigned int pin)
{
	int ret;
	uint32_t val32;
    mv_gpio_prv_t *prv_data = gpiochip_get_data(chip);

	ret = mv_gpio_pcie_map(prv_data);
	if (ret)
		return ret;

    if (pin < 32)
    {
        if (prv_data->reg0->gpio_data_out_ena & (1<<pin))
        {
            // Input
            val32 = !!(prv_data->reg0->gpio_data_in & (1<<pin));
			mv_gpio_pcie_unmap(prv_data);
			return val32;
        }
        else
        {
            // Output
            val32 = !!(prv_data->reg0->gpio_data_out & (1<<pin));
			mv_gpio_pcie_unmap(prv_data);
			return val32;
        }
    }
    else if (pin < 64)
    {
        pin -= 32;
        if (prv_data->reg0->gpio_data_out_ena & (1<<pin))
        {
            // Input
            val32 = !!(prv_data->reg0->gpio_data_in & (1<<pin));
			mv_gpio_pcie_unmap(prv_data);
			return val32;
        }
        else
        {
            // Output
            val32 = !!(prv_data->reg0->gpio_data_out & (1<<pin));
			mv_gpio_pcie_unmap(prv_data);
			return val32;
        }
    }
#if VERBOSE_DBG
    dev_info(prv_data->dev, "%#x:%#x gpio_data_out_0 %#x gpio_data_out_1 %#x\n", 
             prv_data->dev_id->vendor, prv_data->dev_id->device, prv_data->reg0->gpio_data_out, prv_data->reg1->gpio_data_out);
    dev_info(prv_data->dev, "%#x:%#x gpio_data_out_ena_0 %#x gpio_data_out_ena_1 %#x\n", 
             prv_data->dev_id->vendor, prv_data->dev_id->device, prv_data->reg0->gpio_data_out_ena, prv_data->reg1->gpio_data_out_ena);
#endif //VERBOSE_DBG

	mv_gpio_pcie_unmap(prv_data);
    return 0;
}

static void mv_gpio_set(struct gpio_chip *chip, unsigned pin, int val)
{
    int ret;
    mv_gpio_prv_t *prv_data = gpiochip_get_data(chip);

    raw_spin_lock(&prv_data->lock);
    ret = mv_gpio_pcie_map(prv_data);
    if (ret) {
        raw_spin_unlock(&prv_data->lock);
        return;
    }

    if (pin < 32)
    {
        if (val)
        {
            prv_data->reg0->gpio_data_out |= (1<<pin);
        }
        else
        {
            prv_data->reg0->gpio_data_out &= (~(1<<pin));
        }
    }
    else if (pin < 64)
    {
        pin -= 32;
        if (val)
        {
            prv_data->reg1->gpio_data_out |= (1<<pin);
        }
        else
        {
            prv_data->reg1->gpio_data_out &= (~(1<<pin));
        }
    }
    mv_gpio_pcie_unmap(prv_data);
    raw_spin_unlock(&prv_data->lock);
#if VERBOSE_DBG
    dev_info(prv_data->dev, "%#x:%#x gpio_data_out_0 %#x gpio_data_out_1 %#x\n", 
            prv_data->dev_id->vendor, prv_data->dev_id->device, prv_data->reg0->gpio_data_out, 
            prv_data->reg1->gpio_data_out);
    dev_info(prv_data->dev, "%#x:%#x gpio_data_out_ena_0 %#x gpio_data_out_ena_1 %#x\n", 
            prv_data->dev_id->vendor, prv_data->dev_id->device, prv_data->reg0->gpio_data_out_ena, 
            prv_data->reg1->gpio_data_out_ena);
#endif //VERBOSE_DBG

}

static int mv_gpio_set_config(struct gpio_chip *chip, unsigned pin, unsigned long config)
{
    uint32_t pin_conf = pinconf_to_config_param(config);
    switch (pin_conf)
    {
        case PIN_CONFIG_OUTPUT_ENABLE:
        case PIN_CONFIG_OUTPUT:
            mv_gpio_direction_output(chip, pin, 1);
            break;
        case PIN_CONFIG_INPUT_ENABLE:
            mv_gpio_direction_input(chip, pin);
            break;
        default: 
            break;
    }
    return 0;
}

static int mv_platform_config_pin(mv_gpio_prv_t *prv_data, int pin)
{
    struct device *dev = prv_data->dev;
    int ret;
    struct gpio_desc_1 * gdesc_1 = NULL;

    if (pin > GPIO_NPINS)
        return -1;

    prv_data->gdesc[pin] = gpiochip_request_own_desc(&prv_data->chip0, pin, "et6648m-LED"
#if LINUX_VERSION_CODE > KERNEL_VERSION(5, 0, 0)
		    ,GPIO_ACTIVE_HIGH, GPIOD_OUT_HIGH
#endif
		    );
    if (! IS_ERR(prv_data->gdesc[pin]))
    {
        gdesc_1 = (struct gpio_desc_1 *)prv_data->gdesc[pin];
    }
    else
    {
        gdesc_1 = NULL;
        prv_data->gdesc[pin] = NULL;
    }
    dev_info(dev, "%#x:%#x gpiochip_request_own_desc GPIO pin %d(%d) of gpiochip%d gdesc %p label %s name %s flags %#lx\n", 
            prv_data->dev_id->vendor, prv_data->dev_id->device, pin,
            GPIO_PIN_CHIPBASE(pin), prv_data->chip0.base, gdesc_1, gdesc_1->label, gdesc_1->name, gdesc_1->flags);

    if(prv_data->gdesc[pin])
    {
        ret = gpiod_export(prv_data->gdesc[pin], 1);
        if (ret)
        {
            dev_err(dev, "%#x:%#x cannot export pin %d(%d) ret %d\n", prv_data->dev_id->vendor, 
                    prv_data->dev_id->device, pin, GPIO_PIN_CHIPBASE(pin), ret);
            goto cleanup;
        }
        dev_info(dev, "%#x:%#x GPIO pin %d(%d) of gpiochip%d is exported\n", 
                prv_data->dev_id->vendor, prv_data->dev_id->device, pin,
                GPIO_PIN_CHIPBASE(pin), prv_data->chip0.base);

        prv_data->driver_init_state = 4;
        ret = gpiod_export_link(prv_data->dev, mv_gpio0_pin_names[pin], prv_data->gdesc[pin]);
        if (ret)
        {
            dev_err(dev, "%#x:%#x cannot sysfs pin %d(%d) ret %d\n", prv_data->dev_id->vendor, 
                    prv_data->dev_id->device, pin, GPIO_PIN_CHIPBASE(pin), ret);
            goto cleanup;
        }
        dev_info(dev, "%#x:%#x GPIO pin %d(%d) of gpiochip%d linked to sysfs\n", 
                prv_data->dev_id->vendor, prv_data->dev_id->device, pin,
                GPIO_PIN_CHIPBASE(pin), prv_data->chip0.base);
        prv_data->driver_init_state = 5;

        // Default to output
        ret = gpiod_direction_output(prv_data->gdesc[pin], 0);
        dev_info(dev, "%#x:%#x GPIO pin %d(%d) of gpiochip%d set to output\n", 
                prv_data->dev_id->vendor, prv_data->dev_id->device, pin,
                GPIO_PIN_CHIPBASE(pin), prv_data->chip0.base);
    }
cleanup:
    return 0;
}

static int mv_platform_config(mv_gpio_prv_t *prv_data)
{

    if (0 == prv_data->device_id)
    {
         mv_platform_config_pin(prv_data, PSU_LED_GREEN_PIN);
         mv_platform_config_pin(prv_data, PSU_LED_AMBER_PIN);
         mv_platform_config_pin(prv_data, FAN_LED_GREEN_PIN);
         mv_platform_config_pin(prv_data, FAN_LED_AMBER_PIN);
    }
    return 0;
}

static int mv_gpio_probe_pci(struct pci_dev *pdev,
                             const struct pci_device_id *dev_id)
{
    struct device *dev = &pdev->dev;
    mv_gpio_prv_t *prv_data;
    int ret;

	pr_info("%s, version %s.\n", MODULE_NAME,GPIO_DRV_VER);

    prv_data = devm_kzalloc(dev, sizeof(mv_gpio_prv_t), GFP_KERNEL);
    if (!prv_data)
    {
        dev_err(dev, "%s:%s No memmory for private data\n", prv_data->name, MODULE_NAME);
        return -ENOMEM;
    }

	prv_data->pdev = pdev;
    prv_data->dev = dev;
    prv_data->dev_id = dev_id;
    prv_data->name = pci_name(pdev);
    prv_data->driver_init_state = 0;
    prv_data->device_id = driver_ref_count;

    ret = pcim_enable_device(pdev);
    if (ret)
    {
        dev_err(dev, "%s:%s pcim_enable_device ret %d\n", prv_data->name, MODULE_NAME, ret);
        return ret;
    }

    prv_data->driver_init_state = 1;

#if VERBOSE_DBG
    dev_info(dev, "%#x:%#x region mapped resource_0 %p reg0 %p reg1 %p\n", 
             prv_data->dev_id->vendor, prv_data->dev_id->device, 
             (uint8_t *)prv_data->resource_0, prv_data->reg0, prv_data->reg1);
#endif //VERBOSE_DBG

    prv_data->chip0.label = MODULE_NAME;
    prv_data->chip0.parent = prv_data->dev;
    prv_data->chip0.owner = THIS_MODULE;
    prv_data->chip0.base = -1;
    prv_data->chip0.ngpio = GPIO_NPINS;
    if (0 == prv_data->device_id)
    {
        prv_data->chip0.names = mv_gpio0_pin_names;
    }
    else
    {
        prv_data->chip0.names = mv_gpio1_pin_names;
    }
    prv_data->chip0.get_direction = mv_gpio_get_direction;
    prv_data->chip0.direction_input = mv_gpio_direction_input;
    prv_data->chip0.direction_output = mv_gpio_direction_output;
    prv_data->chip0.get = mv_gpio_get;
    prv_data->chip0.set = mv_gpio_set;
    prv_data->chip0.set_config = mv_gpio_set_config;

    raw_spin_lock_init(&prv_data->lock);

    prv_data->driver_init_state = 2;
    ret = devm_gpiochip_add_data(dev, &prv_data->chip0, prv_data);
    if (ret)
    {
        dev_err(dev, "%#x:%#x gpiochip add ret %d\n", prv_data->dev_id->vendor, 
                prv_data->dev_id->device, ret);
        return ret;
    }
    dev_info(dev, "%#x:%#x GPIO %d pins added to gpiochip%d pins(%d to %d)\n", 
             prv_data->dev_id->vendor, prv_data->dev_id->device, 
             GPIO_NPINS, prv_data->chip0.base, prv_data->chip0.base, prv_data->chip0.base+GPIO_NPINS-1);

    prv_data->driver_init_state = 3;
    pci_set_drvdata(pdev, prv_data);

    driver_ref_count++;

    // Board/platform specific config
    mv_platform_config(prv_data);

    return 0;
}

static void mv_gpio_desc_free(mv_gpio_prv_t *prv_data, int pin)
{
    struct gpio_desc * gdesc;
    gdesc = prv_data->gdesc[pin];
    if(gdesc)
    {
        gpiod_unexport(gdesc);
        //gpiod_put(gdesc);
        gpiochip_free_own_desc(gdesc);
    }
}

static void mv_gpio_remove_irqs(struct pci_dev *pdev)
{
    struct msi_desc *entry;
    int i, ii, count;
    struct irq_desc *desc;
    struct irqaction *action;
    unsigned long flags;
    void *dev_id[16];
    u16 control;

    /*
     *	Need to disable MSI before releasing the interrupts, as currently
     *	the Kernel will only write the MSI address with zero WITHOUT
     *	disabling MSI, causing memory overrun of physical address zero in ARM 32-bit:
     */
    pci_read_config_word(pdev, pdev->msi_cap + PCI_MSI_FLAGS, &control);
    control &= ~PCI_MSI_FLAGS_ENABLE;
    pci_write_config_word(pdev, pdev->msi_cap + PCI_MSI_FLAGS, control);

    for_each_pci_msi_entry(entry, pdev)
        if (entry->irq)
            for (i = 0; i < entry->nvec_used; i++)
                if (irq_has_action(entry->irq + i)) {
                    desc = irq_to_desc(entry->irq);

                    if (!desc)
                        continue;

                    count = 0;
                    raw_spin_lock_irqsave(&desc->lock, flags);

                    action = desc->action;
                    while (action) {
                        dev_id[count++] = action->dev_id;
                        action = action->next;
                    }

                    raw_spin_unlock_irqrestore(&desc->lock, flags);
                    for (ii = 0; ii < count; ii++)
                        free_irq(entry->irq, dev_id[ii]);

                }

}

static void mv_gpio_remove_pci(struct pci_dev *pdev)
{
    struct device *dev = &pdev->dev;
    mv_gpio_prv_t *prv_data = pci_get_drvdata(pdev);

	dev_info(dev, "%s: Removing PCI device\n", __func__);
#if 1
    switch (prv_data->driver_init_state)
    {
        case 5:
            if (0 == prv_data->device_id)
            {
                sysfs_remove_link(&dev->kobj,  mv_gpio0_pin_names[PSU_LED_GREEN_PIN]);
                sysfs_remove_link(&dev->kobj,  mv_gpio0_pin_names[PSU_LED_AMBER_PIN]);
                sysfs_remove_link(&dev->kobj,  mv_gpio0_pin_names[FAN_LED_GREEN_PIN]);
                sysfs_remove_link(&dev->kobj,  mv_gpio0_pin_names[FAN_LED_AMBER_PIN]);
            }
        case 4:
            if (0 == prv_data->device_id)
            {
                mv_gpio_desc_free(prv_data, PSU_LED_GREEN_PIN);
                mv_gpio_desc_free(prv_data, PSU_LED_AMBER_PIN);
                mv_gpio_desc_free(prv_data, FAN_LED_GREEN_PIN);
                mv_gpio_desc_free(prv_data, FAN_LED_AMBER_PIN);
            }
        default:
            break;
    }
#endif
	mv_gpio_remove_irqs(pdev);
    dev_info(dev, "%#x:%#x driver removed\n", prv_data->dev_id->vendor, prv_data->dev_id->device);
    pci_set_drvdata(pdev, NULL);
}

static const struct pci_device_id mv_gpio_pci_dev_id[] = {
	{ PCI_DEVICE(PCI_VENDOR_ID_MARVELL, PCI_DEVICE_ID_MRVL_AC3X) }, 
	{ 0 }
};

MODULE_DEVICE_TABLE(pci, mv_gpio_pci_dev_id);

static struct pci_driver mv_gpio_driver = {
	.name = MODULE_NAME,
	.id_table = mv_gpio_pci_dev_id,
	.probe = mv_gpio_probe_pci,
        .remove = mv_gpio_remove_pci
};

module_pci_driver(mv_gpio_driver);

MODULE_AUTHOR("Antony Rheneus <arheneus@marvell.com>");
MODULE_DESCRIPTION("Marvell AC3X gpio Pins");
MODULE_LICENSE("GPL");
