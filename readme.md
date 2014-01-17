
This module add the possibilty to add as many as you want i2c-gpio busses instance
The module create the class gpio_i2c_busses and was create against kernel 3.4.67.
from linux-sunxi git repo and tested on A20-OLinuXino board. But it should work 
on other kernel > 3.4.67 and with other board because nothing specific to A20-OLinuXino
was used.


You can access to the export and unexport function through
The sysfs /sys/class/gpio_i2c_busses/export
          /sys/class/gpio_i2c_busses/unexport


To add a new busses you need to provide 3 params to the export entry. 
 - sda_pin,scl_pin,devid

 ie: echo 49,50,5 > /sys/class/gpio_i2c_busses/expor
 will add a gpio-i2c interface using gpio pin 49 for sda, 50 for scl_pin, device number 5

To remove the busses you need to provide the busses id you provide when export it.

 ie: echo 4 > /sys/class/gpio_i2c_busses/unexport 


How to compile:

if you are using bash

make -C /lib/modules/`uname -r`/build M=$PWD modules

make -C /lib/modules/`uname -r`/build M=$PWD modules_install

cleaning:

make -C /lib/modules/`uname -r`/build M=$PWD clean

if you have any question you can contact me at lemouchon at gmail dot com.

