# OiAK2 Project - A custom kernel module

## Installation

### Do this only once

Prevent hid from taking over every usb device:

`sudo echo 'SUBSYSTEMS=="usb", DRIVERS=="usbhid", ACTION=="add", ATTR{authorized}="0"' > /etc/udev/rules.d/99-disable-hid.rules`

### Do this every time before loading the driver

`sudo rmmod usbhid`

## Do this as often as possible to prevent fuckups

`sudo reboot`

## Knowledge base

[This great howto](http://www.tldp.org/LDP/lkmpg/2.6/html/lkmpg.html#AEN121)


## Building

### Requirements

Required packages (Arch package names, will probably be the same for other distros)

* `g++`
* `linux-headers`

### Compilation & running

`make`
`sudo insmod oiak_modul.ko`
