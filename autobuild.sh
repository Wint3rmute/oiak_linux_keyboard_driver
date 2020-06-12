#!/bin/bash

make || exit 1 

sudo rmmod oiak_modul
sudo rmmod usbhid

sudo insmod oiak_modul.ko