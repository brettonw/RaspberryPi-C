#include "Test.h"
#include "AdafruitServoDriver.h"
#include "Servo.h"
#include "TestDevice.h"
#include "DeviceI2C.h"

TEST_CASE(TestAdafruitServoDriver) {
    //Log::Scope scope (Log::TRACE);
    PtrToTestDevice device = new TestDevice (ADAFRUIT_SERVO_DRIVER_DEFAULT_ADDRESS);
    device
        // PCA9685 constructor
        ->expect (0xfa, 0x00)
        ->expect (0xfb, 0x00)
        ->expect (0xfc, 0x00)
        ->expect (0xfd, 0x00)
        ->expect (0x01, 0x04)
        ->expect (0x00, 0x01)
        ->expect (0x00, 0x00)
        // which calls setPulseFrequency
        ->expect (0x00, (byte) 0x10)
        ->expect (0xfe, (byte) 0x79)
        ->expect (0x00, (byte) 0x00)
        ->expect (0x00, (byte) 0x80);

    PtrTo<AdafruitServoDriver<TestDevice> > driver = new AdafruitServoDriver<TestDevice> (device);
    for (byte i = static_cast<byte> (ServoId::SERVO_00); i <= static_cast<byte> (ServoId::SERVO_15); ++i) {
        TEST_EQUALS(driver->getPulseDuration (static_cast<ServoId>(i)), 0);
    }

    // followed by a call to set channel pulse to set the stop position on servo 1
    device
        ->expect (0x06, (byte) 0x00)
        ->expect (0x07, (byte) 0x00)
        ->expect (0x08, (byte) 0x33)
        ->expect (0x09, (byte) 0x01);

    PtrTo<Servo<AdafruitServoDriver<TestDevice> > > servo0 = new Servo<AdafruitServoDriver<TestDevice> > (driver, ServoId::SERVO_00);
    TEST_EQUALS(driver->getPulseDuration (ServoId::SERVO_00), 1.5);

    // followed by a call to set channel pulse to set the stop position
    device
        ->expect (0x0a, (byte) 0x00)
        ->expect (0x0b, (byte) 0x00)
        ->expect (0x0c, (byte) 0xcd)
        ->expect (0x0d, (byte) 0x00);

    PtrTo<Servo<AdafruitServoDriver<TestDevice> > > servo1 = new Servo<AdafruitServoDriver<TestDevice> > (driver, ServoId::SERVO_01, 0.5, 1.5);
    TEST_EQUALS(driver->getPulseDuration (ServoId::SERVO_01), 1.0);

    TEST_ASSERTION(device->report ());
}
