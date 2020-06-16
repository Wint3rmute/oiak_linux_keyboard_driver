#!/bin/bash

# should be used with `entr` utility:
# `echo oiak_modul.c | entr ./autobuild.sh`

make || exit 1 

sudo rmmod oiak_modul
sudo rmmod usbhid

sudo insmod oiak_modul.ko
