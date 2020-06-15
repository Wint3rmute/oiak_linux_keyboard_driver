# OiAK2 Project - Linux keyboard driver


## Knowledge base

- [Writing kernel modules](http://www.tldp.org/LDP/lkmpg/2.6/html/lkmpg.html#AEN121)
- [Kernel config](https://wiki.gentoo.org/wiki/Kernel/Gentoo_Kernel_Configuration_Guide)
- [Simple keyboard driver source](https://github.com/torvalds/linux/blob/master/drivers/hid/usbhid/usbkbd.c)
- [Writing USB drivers](https://kernel.readthedocs.io/en/sphinx-samples/writing_usb_driver.html)
- [Keyboard key codes](https://gist.github.com/MightyPork/6da26e382a7ad91b5496ee55fdc73db2)


## Building & installation

You can build a standalone kernel module or use a kernel patch to build your very own kernel, with
our keyboard driver built-in.

### Building a kernel module

#### Requirements

##### Required packages (Arch package names, will probably be the same for other distros)

* `g++`
* `linux-headers`


##### You also must prevent `usbhid` from taking over every usb device:

`sudo echo 'SUBSYSTEMS=="usb", DRIVERS=="usbhid", ACTION=="add", ATTR{authorized}="0"' > /etc/udev/rules.d/99-disable-hid.rules`


##### Finally, build and load the module

```
make
sudo insmod oiak_modul.ko
```

### Compiling the kernel with our driver built in

*instructions for Gentoo Linux*

```
# emerge --ask sys-kernel/gentoo-sources
# cd /usr/src/linux
# git init
# git apply /path/to/konami_kbd.diff
# make && make modules_install
```
