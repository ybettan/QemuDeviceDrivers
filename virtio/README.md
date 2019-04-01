# VIRTIO driver

## Overview
This is a simple Virtio device driver that is given as an example of how drivers should be
written.
The device itself is located at https://github.com/ybettan/qemu/blob/virtio/hw/virtio/virtio-example.c
in the [virtio] branch and its purpose is to simply increase a given number by 1.
The driver can be accessed from userspace using sysfs.

## Building instructions
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
    - load: `sudo insmod <path>/QemuDeviceDrivers/virtio/virtio_example_driver.ko`
    - unload: `sudo rmmod virtio_example_driver`
    
## Using instructions
The user can access the driver via sysfs.
  * write: in order to write the number N to the device use
`echo 5 | sudo tee /sys/bus/virtio/devices/virtio1/example/virtio_buf`
    - the user usually don't have permission to access the hw, therefore the 'sudo tee' is a work around for it.
  * read: in order to read the result use `cat /sys/bus/virtio/devices/virtio1/example/virtio_buf`
