#Raspberry Pi

## GPIO references
* https://github.com/raspberrypi/documentation/blob/master/hardware/computemodule/README.md
* http://wiringpi.com/
* http://abyz.me.uk/rpi/pigpio/index.html
* http://www.hertaville.com/introduction-to-accessing-the-raspberry-pis-gpio-in-c.html
* https://elinux.org/RPi_Low-level_peripherals#GPIO_Driving_Example_.28C.29
* http://developer.ridgerun.com/wiki/index.php?title=How_to_use_GPIO_signals
* https://github.com/halherta/RaspberryPi-GPIOClass-v2
* http://www.airspayce.com/mikem/bcm2835/
* https://github.com/RPi-Distro/raspi-gpio/blob/master/raspi-gpio.c
* https://elinux.org/RPi_BCM2835_GPIOs
* https://pinout.xyz/

This one in particular was useful:
* http://abyz.me.uk/rpi/pigpio/examples.html#Misc_tiny_gpio

The user running the program should be a member of the "gpio" group.
````
sudo usermod -a -G gpio <username>
````
...and the user should log out and then back in.


