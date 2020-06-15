# OiAK2 Project - Linux keyboard driver


## Knowledge base

- [tldp](http://www.tldp.org/LDP/lkmpg/2.6/html/lkmpg.html#AEN121)


## Building & installation

### As kernel module

#### Requirements

Required packages (Arch package names, will probably be the same for other distros)

* `g++`
* `linux-headers`


You also must prevent `usbhid` from taking over every usb device:

`sudo echo 'SUBSYSTEMS=="usb", DRIVERS=="usbhid", ACTION=="add", ATTR{authorized}="0"' > /etc/udev/rules.d/99-disable-hid.rules`


Finally, build and load the module

`make`

`sudo insmod oiak_modul.ko`

### As a builtin kernel driver

*instructions for Gentoo Linux*

```
# emerge --ask sys-kernel/gentoo-sources
# cd /usr/src/linux
# git init
# git apply /path/to/konami_kbd.diff
# make && make modules_install
```
