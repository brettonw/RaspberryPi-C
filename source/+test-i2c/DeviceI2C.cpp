#include "Test.h"
#include "DeviceI2C.h"
#include "AdafruitMotorDriver.h"

TEST_CASE(TestDeviceI2C) {
    try {
        //Log::Scope scope (Log::TRACE);
        DeviceI2C   device (ADAFRUIT_MOTOR_DRIVER_DEFAULT_ADDRESS);

        // this test checks whether the device is active and responding by writing a register and
        // confirming that value. The register, 0x44 is a high LED OFF register on the adafruit
        // motor driver - so it can be set to a range of values. if you try to set the ON register,
        // it will fail (silently) if the value is greater than the corresponding off register.
        // XXX TODO: find a general way to do this check across all devices
        device.begin ();
        device.write (0x44, 0x10);
        Pause::milli (10);
        int x = device.read (0x44);
        TEST_EQUALS(x, 0x10);
        Pause::milli (10);

        device.write (0x44, 0x11);
        Pause::milli (10);
        x = device.read (0x44);
        TEST_EQUALS(x, 0x11);
        Pause::milli (10);

        device.write (0x44, 0x20);
        Pause::milli (10);
        x = device.read (0x44);
        TEST_EQUALS(x, 0x20);
        device.end ();

        Pause::milli (1000);
    }
    catch (RuntimeError& runtimeError) {
        Log::exception (runtimeError);
        TEST_TRUE(true);
    }
}
