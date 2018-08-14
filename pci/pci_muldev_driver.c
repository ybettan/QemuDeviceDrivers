#include <linux/module.h>
#include <linux/device.h>
#include <linux/pci.h>
#include <linux/interrupt.h>
#include <asm/io.h>             /* io map */
#include <linux/dma-mapping.h>  /* DMA */


#define PCI_VENDOR_ID_REDHAT 0x1b36
#define PCI_DEVICE_ID_REDHAT_MULDEV 0x0005

#define DMA_BUFFER_SIZE 4096


/* pointers to handle IO\MEM\IRQ\DMA read\write */ 
static void __iomem *io, *mem, *irq, *dmaBase;

/* kobject for sysfs use */
static struct kobject *muldevKob;
    
/* implementation is at the buttom */
static struct pci_driver muldev;

/* for IRQ support - handler write the result here when IRQ fired */
uint64_t ioData, memData;

/* for DMA support */
void *dmaBuffVirtualAdd;
dma_addr_t dmaBuffPhysicalAddr;


//-----------------------------------------------------------------------------
//                  sysfs - give user access to driver
//-----------------------------------------------------------------------------

static ssize_t muldev_show(struct kobject *kobj, struct kobj_attribute *attr,
                              char *buf);
static ssize_t muldev_store(struct kobject *kobj, struct kobj_attribute *attr,
                              const char *buf, size_t count);

/* 
 * define a new attribute (a "file" in sysfs) type when _ATTR is:
 * #define __ATTR(_name,_mode,_show,_store) { \
 *     .attr = { \
 *         .name = __stringify(_name), \
 *         .mode = _mode \
 *     }, \
 *     .show = _show, \
 *     .store= _store, \
 * } 
 */
struct kobj_attribute muldev_attr_io = 
    __ATTR(io_buff, 0664, muldev_show, muldev_store);

struct kobj_attribute muldev_attr_mem = 
    __ATTR(mem_buff, 0664, muldev_show, muldev_store);



/* 
 * The muldev_attr defined above is then grouped in the struct attribute group 
 * as follows:
 */
struct attribute *muldev_attrs[] = {
    &muldev_attr_io.attr,
    &muldev_attr_mem.attr,
    NULL,
};


/* The above attributes are then given to the attribute group as follows: */
struct attribute_group muldev_attr_group = {
    .attrs = muldev_attrs,
};



static ssize_t muldev_show(struct kobject *kobj, struct kobj_attribute *attr,
                              char *buf)
{
    if (attr == &muldev_attr_io) {
        return sprintf(buf, "%llu\n", ioData);
    }

    if (attr == &muldev_attr_mem) {
        return sprintf(buf, "%llu\n", memData);
    }

    return -EPERM;
}

static ssize_t muldev_store(struct kobject *kobj, struct kobj_attribute *attr,
                              const char *buf, size_t count)
{
    char num = *buf - '0';

    if (attr == &muldev_attr_io) {

        /* invalidate the old data and write the new one*/
        ioData = -1;
        iowrite8(num, io); 

        /* ehco 3 > filename is trying to write 2 chars {'3', ' '} */
        return 2; 
    }

    if (attr == &muldev_attr_mem) {

        /* invalidate the old data and write the new one*/
        memData = -1;
        iowrite8(num, mem); 

        /* ehco 3 > filename is trying to write 2 chars {'3', ' '} */
        return 2; 
    }

    return -EPERM;
}



//-----------------------------------------------------------------------------
//                              IRQ functions
//-----------------------------------------------------------------------------

static irqreturn_t muldev_irq_handler(int irqNum, void *devId)
{
    if (ioread8(irq)) {

        /* copy the data from relevant pipes to local variable - will wait
         * there until the user will read the values from there */
        ioData = ioread8(io);
        memData = *(char*)dmaBuffVirtualAdd;

        /* deassert IRQ */
        iowrite8(0, irq);
        return IRQ_HANDLED;

    } else {

        /* if the IRQ port is 0 than another device caused the IRQ */
        return IRQ_NONE;
    }
}


//-----------------------------------------------------------------------------
//                             driver functions
//-----------------------------------------------------------------------------

static int muldev_probe(struct pci_dev *dev, const struct pci_device_id *id)
{
    u8 irqNum;
    uint32_t upperBytesAddr, lowerBytesAddr;

    /* enabling the device */
    if (pci_enable_device(dev)) {
        pr_alert("failed to enable the device\n");
    }

    /* map pci BAR addresses to CPU addresses */
    if (pci_request_regions(dev, "muldev")) {
        pr_alert("failed to map pci BAR addresses to CPU addresses\n");
    }

    /* map CPU adress space to kernel virtual addresses */
    mem = pci_iomap(dev, 0, 1);
    io = pci_iomap(dev, 1, 1);
    irq = pci_iomap(dev, 2, 1);
    dmaBase = pci_iomap(dev, 3, 1);

    /* get device IRQ number */
    if(pci_read_config_byte(dev, PCI_INTERRUPT_LINE, &irqNum)) {
        pr_alert("failed to get IRQ number\n");
    }

    /* irq registeration */
    if (devm_request_irq(&dev->dev, irqNum, muldev_irq_handler, IRQF_SHARED, 
                muldev.name, "")) {
        pr_alert("failed to register irq and its handler\n");
    }

    /* setting up coherent DMA mapping */
    dmaBuffVirtualAdd = dma_alloc_coherent(&dev->dev, DMA_BUFFER_SIZE, 
            &dmaBuffPhysicalAddr, GFP_KERNEL);

    /* give the device the base address of the DMA-buffer allocated */
    lowerBytesAddr = dmaBuffPhysicalAddr & 0x00000000ffffffff;
    upperBytesAddr = dmaBuffPhysicalAddr >> 32;
    iowrite32(lowerBytesAddr, dmaBase); 
    iowrite32(upperBytesAddr, (void*)((char*)dmaBase + sizeof(uint32_t))); 

    /* creates a directory inside /sys/kernel for user, kobj = sysfs_dir */
    muldevKob = kobject_create_and_add("muldev", kernel_kobj);
    if (!muldevKob) {
        pr_alert("failed to create a /sys/kernel directory for user\n");
    }

    /* create sysfiles for user communication */
    if (sysfs_create_group(muldevKob, &muldev_attr_group)) {
        pr_alert("failed to create sysfiles in /sys/kernel/dirname for user\n");
    }

    return 0;
}

static void muldev_remove(struct pci_dev *dev)
{
    //FIXME: call with the correct params
    //devm_free_irq(dev);
    dma_free_coherent(&dev->dev, DMA_BUFFER_SIZE, dmaBuffVirtualAdd,
            dmaBuffPhysicalAddr);
    pci_release_regions(dev);
    pci_disable_device(dev);
    pci_iounmap(dev, io);
    pci_iounmap(dev, mem);
    pci_iounmap(dev, irq);
    pci_iounmap(dev, dmaBase);

    /* remove the directory from sysfs */
    kobject_del(muldevKob);
}

/* vendor and device (+ subdevice and subvendor)
 * identifies a device we support
 */
static struct pci_device_id muldev_ids[] = {

    { PCI_DEVICE(PCI_VENDOR_ID_REDHAT, PCI_DEVICE_ID_REDHAT_MULDEV) },
    { 0, },
};

/* id_table describe the device this driver support
 * probe is called when a device we support exist and
 * when we are chosen to drive it.
 * remove is called when the driver is unloaded or
 * when the device disappears
 */
static struct pci_driver muldev = {
    .name = "muldev",
    .id_table = muldev_ids,
    .probe = muldev_probe,
    .remove = muldev_remove,
};



//-----------------------------------------------------------------------------
//                          overhead - must have
//-----------------------------------------------------------------------------



/* register driver in kernel pci framework */
module_pci_driver(muldev);
MODULE_DEVICE_TABLE(pci, muldev_ids);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Muldev");
MODULE_AUTHOR("Yoni Bettan");


