#include <linux/virtio.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/pci.h>
#include <linux/interrupt.h>
#include <asm/io.h>             /* io map */
#include <linux/dma-mapping.h>  /* DMA */
#include <linux/kernel.h>       /* kstrtoint() func */
#include <linux/virtio_config.h>       /* find_single_vq() func */


#define VIRTIO_ID_EXAMPLE 21

#define DMA_BUF_SIZE 4096
#define BYTE_MAX_SIZE 255


///* pointers to handle IO\MEM\IRQ\DMA read\write */
//static void __iomem *io, *mem, *irq, *dma_base;

///* implementation is at the buttom */
//static struct pci_driver example;

/* for IRQ support - handler write the result here when IRQ fired */
//uint64_t io_data, mem_data;
//uint64_t virtio_data;
//uint8_t data_to_device, data_from_device;

///* for DMA support */
//void *dma_buf_virtual_addr;
//dma_addr_t dma_buf_physical_addr;
//
//FIXME: change to non-global
struct virtqueue *vq;

//-----------------------------------------------------------------------------
//                  sysfs - give user access to driver
//-----------------------------------------------------------------------------

static ssize_t
virtio_buf_store(struct device *dev, struct device_attribute *attr,
        const char *buf, size_t count)
{
	struct scatterlist sg;

    sg_init_one(&sg, buf, count);

	/* There should always be room for one buffer. */
	virtqueue_add_inbuf(vq, &sg, 1, buf, GFP_KERNEL);

    /* notify the device */
	virtqueue_kick(vq);

    pr_alert("virtio_buf_store()\n");

    return count;
}

static ssize_t
virtio_buf_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    pr_alert("virtio_buf_show()\n");
    return 1;
}

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
static DEVICE_ATTR_RW(virtio_buf);


/*
 * The example_attr defined above is then grouped in the struct attribute group
 * as follows:
 */
struct attribute *example_attrs[] = {
    &dev_attr_virtio_buf.attr,
    NULL,
};

static const struct attribute_group example_attr_group = {
    .name = "example", /* directory's name */
    .attrs = example_attrs,
};



//static ssize_t example_show(struct kobject *kobj, struct kobj_attribute *attr,
//                              char *buf)
//{
//    //if (attr == &example_attr_io) {
//    //    return sprintf(buf, "%llu\n", io_data);
//    //}
//
//    //if (attr == &example_attr_mem) {
//    //    return sprintf(buf, "%llu\n", mem_data);
//    //}
//
//    //return -EPERM;
//    //
//
//    //return sprintf(buf, "%u\n", data_from_device);
//    return 1;
//}

//static ssize_t example_store(struct kobject *kobj, struct kobj_attribute *attr,
//                              const char *buf, size_t size)
//{
//    int num;
//	struct scatterlist sg;
//
//    /* convert buffer to byte */
//    if (kstrtoint(buf, 10, &num)) {
//        pr_alert("faild to convert the input into a byte on write\n");
//    }
//
//    /* only number <= 254 are supported (the result should be 1 byte max) */
//    if (num > BYTE_MAX_SIZE-1) {
//        pr_alert("supports only numbers in range [0:254] - 1 byte size");
//    }
//
//    //data_to_device = num;
//
//    pr_alert("num to store is %d\n", num);
//    //{
//    //    char *buf2 = kmalloc(5, GFP_KERNEL);
//	//    sg_init_one(&sg, buf2, size);
//    //    kfree(buf2);
//    //}
//
//	///* There should always be room for one buffer. */
//	//virtqueue_add_inbuf(vq, &sg, 1, buf, GFP_KERNEL);
//
//    ///* notify the device */
//	//virtqueue_kick(vq);
//
////
////    if (attr == &example_attr_io) {
////
////        /* invalidate the old data and write the new one*/
////        io_data = 0;
////        iowrite8(num, io);
////
////        /* ehco 3 > filename is trying to write 2 chars {'3', ' '} */
////        return size;
////    }
////
////    if (attr == &example_attr_mem) {
////
////        /* invalidate the old data and write the new one*/
////        mem_data = 0;
////        iowrite8(num, mem);
////
////        /* ehco 3 > filename is trying to write 2 chars {'3', ' '} */
////        return size;
////    }
////
////    return -EPERM;
//        return size;
//}



//-----------------------------------------------------------------------------
//                              IRQ functions
//-----------------------------------------------------------------------------

static void example_irq_handler(struct virtqueue *vq)
{
    //if (ioread8(irq)) {

    //    /*
    //     * copy the data from relevant pipes to local variable - will wait
    //     * there until the user will read the values from there
    //     */
    //    io_data = ioread8(io);
    //    mem_data = *(uint64_t*)dma_buf_virtual_addr;

    //    /* deassert IRQ */
    //    iowrite8(0, irq);
    //    return IRQ_HANDLED;

    //} else {

    //    /* if the IRQ port is 0 than another device caused the IRQ */
    //    return IRQ_NONE;
    //}
}


//-----------------------------------------------------------------------------
//                             driver functions
//-----------------------------------------------------------------------------


//static int example_probe(struct pci_dev *dev, const struct pci_device_id *id)
static int example_probe(struct virtio_device *vdev)
{
    //u8 irq_num;
    //uint32_t upper_bytes_addr, lower_bytes_addr;
    int retval;


    /* create sysfiles for UI */
    retval = sysfs_create_group(kernel_kobj, &example_attr_group);
    if (retval) {
        pr_alert("failed to create group in /sys/kernel/\n");
    }

    //FIXME: add error check
	/* We expect a single virtqueue. */
    pr_alert("before: vq = %p\n", vq);
	vq = virtio_find_single_vq(vdev, example_irq_handler, "input");
    pr_alert("after: vq = %p\n", vq);
	//if (IS_ERR(vi->vq)) {
	//	err = PTR_ERR(vi->vq);
	//	goto err_find;
	//}


    /* get device IRQ number */
    //if(pci_read_config_byte(dev, PCI_INTERRUPT_LINE, &irq_num)) {
    //    pr_alert("failed to get IRQ number\n");
    //}

    /* irq registeration */
    //if (devm_request_irq(&dev->dev, irq_num, example_irq_handler, IRQF_SHARED,
    //            example.name, "")) {
    //    pr_alert("failed to register irq and its handler\n");
    //}

    /* setting up coherent DMA mapping */
    //dma_buf_virtual_addr = dma_alloc_coherent(&dev->dev, DMA_BUF_SIZE,
    //        &dma_buf_physical_addr, GFP_KERNEL);

    /* give the device the base address of the DMA-buffer allocated */
    //lower_bytes_addr = dma_buf_physical_addr & 0x00000000ffffffff;
    //upper_bytes_addr = dma_buf_physical_addr >> 32;
    //iowrite32(lower_bytes_addr, dma_base);
    //iowrite32(upper_bytes_addr, (void*)((char*)dma_base + sizeof(uint32_t)));

    pr_alert("virtio EXAMPLE driver was loaded\n");

    return 0;
}

//static void example_remove(struct pci_dev *dev)
static void example_remove(struct virtio_device *vdev)
{
    ////FIXME: call with the correct params
    ////devm_free_irq(dev);
    //dma_free_coherent(&dev->dev, DMA_BUF_SIZE, dma_buf_virtual_addr,
    //        dma_buf_physical_addr);

    /* remove the directory from sysfs */
    //kobject_del(example_kobj);
    sysfs_remove_group(kernel_kobj, &example_attr_group);

    /* disable interrupts for vqs */
    vdev->config->reset(vdev);

    /* remove virtqueues */
	vdev->config->del_vqs(vdev);

    pr_alert("virtio EXAMPLE driver was removed\n");
}

//static void virtrng_scan(struct virtio_device *vdev)
//{
//	struct virtrng_info *vi = vdev->priv;
//	int err;
//
//	err = hwrng_register(&vi->hwrng);
//	if (!err)
//		vi->hwrng_register_done = true;
//}

/* vendor and device (+ subdevice and subvendor)
 * identifies a device we support
 */
static struct virtio_device_id example_ids[] = {

    {
        .device = VIRTIO_ID_EXAMPLE,
        .vendor = VIRTIO_DEV_ANY_ID,
    },
    { 0, },
};

/* id_table describe the device this driver support
 * probe is called when a device we support exist and
 * when we are chosen to drive it.
 * remove is called when the driver is unloaded or
 * when the device disappears
 */
static struct virtio_driver example = {
	.driver.name =	"example",
	.driver.owner =	THIS_MODULE,
	.id_table =	example_ids,
	.probe =	example_probe,
	.remove =	example_remove,
};



//-----------------------------------------------------------------------------
//                          overhead - must have
//-----------------------------------------------------------------------------



/* register driver in kernel pci framework */
module_virtio_driver(example);
MODULE_DEVICE_TABLE(virtio, example_ids);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Example virtio");
MODULE_AUTHOR("Yoni Bettan");

