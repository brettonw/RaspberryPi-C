#include "Pin.h"

/*
the Raspberry Pi maps these pins to a 40-pin J-8 header, and only exposes some of the real GPIO
pins, I adapted the following diagram from several references:

    http://www.cpmspectrepi.uk/raspberry_pi/MoinMoinExport/WiringPiGpioReadall.html
    https://pinout.xyz/
    https://www.raspberrypi.org/forums/download/file.php?id=20369

Note: I removed WiringPi references because they are not relevant to me

 +-----+-----------+------+---+---Pi 3---+---+------+---------+-----+
 | BCM |    Name   | Mode | V | Physical | V | Mode |  Name   | BCM |
 +-----+-----------+------+---+----++----+---+------+---------+-----+
 |     |     3.3v  |      |   |  1 || 2  |   |      | 5v      |     |
 |   2 |  I2C SDA1 | ALT0 | 1 |  3 || 4  |   |      | 5V      |     |
 |   3 |  I2C SCL1 | ALT0 | 1 |  5 || 6  |   |      | GND     |     |
 |   4 | GPIO_GCLK |   IN | 1 |  7 || 8  | 1 | ALT5 | TxD     | 14  |
 |     |       GND |      |   |  9 || 10 | 1 | ALT5 | RxD     | 15  |
 |  17 |    GPIO_0 |   IN | 0 | 11 || 12 | 0 | IN   | GPIO_01 | 18  |
 |  27 |    GPIO_2 |   IN | 0 | 13 || 14 |   |      | GND     |     |
 |  22 |    GPIO_3 |   IN | 0 | 15 || 16 | 0 | IN   | GPIO_04 | 23  |
 |     |      3.3v |      |   | 17 || 18 | 0 | IN   | GPIO_05 | 24  |
 |  10 |  SPI_MOSI | ALT0 | 0 | 19 || 20 |   |      | GND     |     |
 |   9 |  SPI_MISO | ALT0 | 0 | 21 || 22 | 0 | IN   | GPIO_06 | 25  |
 |  11 |      SCLK | ALT0 | 0 | 23 || 24 | 1 | OUT  | CE0     | 8   |
 |     |       GND |      |   | 25 || 26 | 1 | OUT  | CE1     | 7   |
 |   0 |     ID_SD |   IN | 1 | 27 || 28 | 1 | IN   | SCL_0   | 1   |
 |   5 |   GPIO_21 |   IN | 1 | 29 || 30 |   |      | GND     |     |
 |   6 |   GPIO_22 |   IN | 1 | 31 || 32 | 0 | IN   | GPIO_26 | 12  |
 |  13 |   GPIO_23 |   IN | 0 | 33 || 34 |   |      | GND     |     |
 |  19 |   GPIO_24 |   IN | 0 | 35 || 36 | 0 | IN   | GPIO_27 | 16  |
 |  26 |   GPIO_25 |   IN | 0 | 37 || 38 | 0 | IN   | GPIO_28 | 20  |
 |     |       GND |      |   | 39 || 40 | 0 | IN   | GPIO_29 | 21  |
 +-----+-----------+------+---+----++----+---+------+---------+-----+
 | BCM |    Name   | Mode | V | Physical | V | Mode | Name    | BCM |
 +-----+-----------+------+---+---Pi 3---+---+------+---------+-----+

Some of the GPIO pins are dedicated to "alternate functions" on the Raspberry Pi, see:

    https://www.raspberrypi.org/documentation/usage/gpio/

- PWM (pulse-width modulation)
    Hardware PWM available on GPIO_12, GPIO_13, GPIO_18, GPIO_19

- SPI
    SPI0: MOSI (GPIO_10); MISO (GPIO_09); SCLK (GPIO_11); CE0 (GPIO_08), CE1 (GPIO_07)
    SPI1: MOSI (GPIO_20); MISO (GPIO_19); SCLK (GPIO_21); CE0 (GPIO_18); CE1 (GPIO_17); CE2 (GPIO_16)

- I2C
    Data: (GPIO2); Clock (GPIO3)
    EEPROM Data: (GPIO0); EEPROM Clock (GPIO1)

- Serial
    TX (GPIO14); RX (GPIO15)
*/

int mapRpiPinToGpioPin[RASPBERRY_PI_PIN_COUNT] = {
GPIO_XX, GPIO_XX, // 01, 02
GPIO_02, GPIO_XX, // 03, 04
GPIO_03, GPIO_XX, // 05, 06
GPIO_04, GPIO_14, // 07, 08
GPIO_XX, GPIO_15, // 09, 10
GPIO_17, GPIO_18, // 11, 12
GPIO_27, GPIO_XX, // 13, 14
GPIO_22, GPIO_23, // 15, 16
GPIO_XX, GPIO_24, // 17, 18
GPIO_10, GPIO_XX, // 19, 20
GPIO_09, GPIO_25, // 21, 22
GPIO_11, GPIO_08, // 23, 24
GPIO_XX, GPIO_07, // 25, 26
GPIO_00, GPIO_01, // 27, 28
GPIO_05, GPIO_XX, // 29, 30
GPIO_06, GPIO_12, // 31, 32
GPIO_13, GPIO_XX, // 33, 34
GPIO_19, GPIO_16, // 35, 36
GPIO_26, GPIO_20, // 37, 38
GPIO_XX, GPIO_21, // 39, 40
};
