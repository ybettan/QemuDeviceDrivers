# QemuDeviceDrivers
Here I will keep all the drivers written for all the Qemu devices I wrote

## PCI driver

#### Overview
This is a simple PCI device driver that is given as an example of how drivers should be
written.
The device itself is located at https://github.com/ybettan/qemu/tree/pci/hw/pci/pci_muldev.c
in the [pci] branch and its purpose is to simply multiply a given number.
The device supports PORTIO, MMIO, IRQ and DMA and the driver can be accessed from
userspace using sysfs.

#### Building instructions
1. check the running kernel version with
    - `uname -r`
2. check that you have the kernel source code for this version
    - `ls lib/modules/$(uname -r)/build/`
    - if the folder isn't empty you have the source code
    - if it is empty install it using `sudo dnf install kernel-devel`
3. once you have the source code make sure your kernel is up to date
    - it means your running kernel and your kernel source code versions matches
    - if the version don't match update your kernel with `sudo dnf update` and reboot
4. compile the module
    - from QemuDeviceDrivers/pci/ run `make -C /lib/modules/$(uname -r)/build M=$PWD modules`
5. now you can load and unload the driver
    - load: `sudo insmod <path>/QemuDeviceDrivers/pci/pci_muldev_driver.ko`
    - unload: `sudo rmmod pci_muldev_driver`
