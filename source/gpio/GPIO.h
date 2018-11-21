#pragma once

#include "Log.h"
#include "RuntimeError.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/mman.h>

/*
This documentation is abstracted from the BCM2835 and BCM2837 datasheets, see chapter 6 for GPIO
explanations, and the addenda from the Raspberry Pi Foundation: rpi_DATA_CM_2p0.pdf, Chapter 9.

- GPIO
There are 54 general-purpose I/O (GPIO) lines split into two banks. The banks are not well
documented, but are presumably (0-31) and (32-53).

- GPIO Registers
The GPIO has 41 registers. All accesses are assumed to be 32-bit. The registers we care about are:

    Index | Offset | Name    | Description       | R/W | Pin(s)
    ------+--------+---------+-------------------+-----+--------
     0x00   0x0000   GPFSEL0   Function Select 0   R/W    0-9
     0x01   0x0004   GPFSEL1   Function Select 1   R/W   10-19
     0x02   0x0008   GPFSEL2   Function Select 2   R/W   20-29
     0x03   0x000C   GPFSEL3   Function Select 3   R/W   30-39
     0x04   0x0010   GPFSEL4   Function Select 4   R/W   40-49
     0x05   0x0014   GPFSEL5   Function Select 5   R/W   50-53
     0x06   0x0018             - Reserved - -
     0x07   0x001C   GPSET0    Pin Output Set 0     W     0-31
     0x08   0x0020   GPSET1    Pin Output Set 1     W    32-53
     0x09   0x0024             - Reserved - -
     0x0a   0x0028   GPCLR0    Pin Output Clear 0   W     0-31
     0x0b   0x002C   GPCLR1    Pin Output Clear 1   W    32-53
     0x0c   0x0030             - Reserved - -
     0x0d   0x0034   GPLEV0    Pin Level 0          R     0-31
     0x0e   0x0038   GPLEV1    Pin Level 1          R    32-53

(Adapted from Table 6-1, which outlines a total reserved address space of 180 bytes)

- GPIO Function Select Registers (GPFSELn)
The function select registers are used to define the operation of the general-purpose I/O
pins. Each of the 54 GPIO pins has at least two alternative functions as defined in section
16.2. The FSEL{n} field determines the functionality of the nth GPIO pin. All unused
alternative function lines are tied to ground and will output a 0 if selected. All pins reset
to normal GPIO input operation.

Each 32-bit GPFSEL register controls 10 GPIO pins, using a 3 bit code per pin:
    000 = GPIO Pin is an input
    001 = GPIO Pin is an output
    100 = GPIO Pin takes alternate function 0
    101 = GPIO Pin takes alternate function 1
    110 = GPIO Pin takes alternate function 2
    111 = GPIO Pin takes alternate function 3
    011 = GPIO Pin takes alternate function 4
    010 = GPIO Pin takes alternate function 5

- GPIO Pin Output Set Registers (GPSETn)
The output set registers are used to set a GPIO pin. The SET{n} field defines the
respective GPIO pin to set, writing a 0 to the field has no effect. If the GPIO pin is
being used as in input (by default) then the value in the SET{n} field is ignored.
However, if the pin is subsequently defined as an output then the bit will be set
according to the last set/clear operation. Separating the set and clear functions
removes the need for read-modify-write operations.

- GPIO Pin Output Clear Registers (GPCLRn)
The output clear registers are used to clear a GPIO pin. The CLR{n} field defines
the respective GPIO pin to clear, writing a 0 to the field has no effect. If the GPIO
pin is being used as in input (by default) then the value in the CLR{n} field is
ignored. However, if the pin is subsequently defined as an output then the bit will
be set according to the last set/clear operation. Separating the set and clear
functions removes the need for read-modify-write operations.

- GPIO Pin Level Registers (GPLEVn)
The pin level registers return the actual value of the pin. The LEV{n} field gives the
value of the respective GPIO pin.
*/

#include        "Pin.h"

MAKE_PTR_TO(GPIO) {
    private:
        volatile uint* registers;

        GPIO* op (uint registerBase, Pin pin) {
            // there are two banks (0 or 1) of pins (0-31 and 32-53), so we divide by 32 to
            // determine which register to fetch, and mask out all but the lower 5 bits (which is
            // equivalent to a modulo 32) to compute the offset within the register.
            uint which = registerBase + (pin >> 5);
            uint offset = pin & BANK_MASK;
            registers[which] = (0x01 << offset);
            return this;
        }

        enum {
            // register offsets
            GPFSEL = 0x00,
            GPSET = 0x07,
            GPCLR = 0x0a,
            GPLEV = 0x0d,

            // 3-bit mask for working with function select registers
            FSEL_MASK = 0x07,

            // 5 bit mask for working with register banks
            BANK_MASK = 0x1f,

            // total GPIO register address space, 180 bytes
            GPIO_BLOCK_SIZE = 0xb4
        };

    public:
        GPIO (const char* fileToMap = "/dev/gpiomem", uint baseAddress = 0) {
            // map the requested file and base address to a user-space address - raspberry pi
            // specifically exports the "/dev/gpiomem" file for a base address of 0, but it might be
            // different for another platform. you could also use "/dev/mem" and map the base
            // address. The datasheet for BCM2835 specifies a bus address of 0x7e200000, but the
            // Raspberry Pi maps this to physical memory at 0x20200000, and later models map it to
            // 0x3f200000, so either of the latter two values would be the base address to use,
            // depending on what model you are running.
            int fd = open(fileToMap, O_RDWR | O_SYNC) ;
            if (fd >= 0) {
                Log::debug () << "GPIO: " << "opened map file at " << fileToMap << endl;
                registers = static_cast<uint*> (mmap (0, GPIO_BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, baseAddress));
                close(fd);

                if (registers == MAP_FAILED) {
                    throw RuntimeError (Text("GPIO: ") << "can't map GPIO registers");
                }
                Log::info () << "GPIO: " << "ready to talk" << endl;
            } else {
                throw RuntimeError (Text("GPIO: ") << "can't open map file at " << fileToMap);
            }
        }

        ~GPIO () {
            munmap ((void*) registers, GPIO_BLOCK_SIZE);
            Log::info () << "GPIO: " << "released map" << endl;
        }

        // data types and methods for setting and getting pin functions
        enum Function {
            INPUT = 0x00, OUTPUT = 0x01,
            ALTERNATE_0 = 0x04, ALTERNATE_1 = 0x05, ALTERNATE_2 = 0x06,
            ALTERNATE_3 = 0x07, ALTERNATE_4 = 0x03, ALTERNATE_5 = 0x02
        };

        GPIO* setFunction (Pin pin, Function function) {
            // there are 6 function select registers, each controls 10 GPIO pins, with 3 bits for
            // each pin. we divide by 10 to determine which register to use (0-5), and then use the
            // modulo operator to get the offset within the register. we first mask out the 3 bits
            // that apply to the selected pin, and then 'or' the new bits in.
            uint which =  GPFSEL + (pin / 10);
            uint offset = (pin % 10) * 3;
            registers[which] = (registers[which] & ~(FSEL_MASK << offset)) | (function << offset);
            return this;
        }

        Function getFunction (Pin pin) {
            // there are 6 function select registers (0-5) starting at the base address, each
            // controls 10 GPIO pins, with 3 bits for each pin. we divide by 10 to determine which
            // register to fetch, and then use the modulo operator to compute the offset within the
            // register. finally, we mask off any bits that might apply to higher pins.
            uint which =  GPFSEL + (pin / 10);
            uint offset = (pin % 10) * 3;
            return static_cast<Function>((registers[which] >> offset) & FSEL_MASK);
        }

        GPIO* getFunction (Pin pin, Function* function) {
            *function = getFunction (pin);
            return this;
        }

        // methods for setting and clearing pins that are functioning as outputs
        GPIO* set (Pin pin) {
            return op (GPSET, pin);
        }

        GPIO* clear (Pin pin) {
            return op (GPCLR, pin);
        }

        GPIO* write (Pin pin, bool high) {
            return high ? set (pin) : clear (pin);
        }

        // method for getting the current level of a pin - this works regardless of the pin function
        bool get (Pin pin) {
            // there are two banks (0 or 1) of pins (0-31 and 32-53), so we divide by 32 to
            // determine which register to fetch, and mask out all but the lower 5 bits (which is
            // equivalent to a modulo 32) to compute the offset within the register.
            uint which = GPLEV + (pin >> 5);
            uint offset = pin & BANK_MASK;
            return (registers[which] & (0x01 << offset)) ? true : false;
        }

        GPIO* get (Pin pin, bool* value) {
            *value = get (pin);
            return this;
        }

        // and the typical combination of get/set
        GPIO* toggle (Pin pin) {
            //return get (pin) ?  clear (pin) : set (pin);
            return write (pin, not get (pin));
        }
};
