#!/bin/sh

for DEV in /sys/bus/usb/devices/*
do
if [ -e "$DEV/bDeviceClass" ]
then
	CLASS=`cat "$DEV/bDeviceClass"`

	if [ "$CLASS" = "00" ] || [ "$CLASS" = "08" ]
	then
		PRODUCT=`cat "$DEV/product"`
		SERIAL=`cat "$DEV/serial"`

		echo "serial=$SERIAL product=$PRODUCT"
	fi
fi

done
