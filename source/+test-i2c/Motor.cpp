#include "Test.h"
#include "AdafruitMotorDriver.h"
#include "NullDevice.h"
#include "DeviceI2C.h"
#include "Motor.h"

TEST_CASE(TestAMotor) {
    //Log::Scope scope (Log::TRACE);

    PtrToNullDevice device = new NullDevice ();
    PtrTo<AdafruitMotorDriver<NullDevice> > driver = new AdafruitMotorDriver<NullDevice> (device);
    for (byte i = 0; i < MOTOR_COUNT; ++i) {
        TEST_EQUALS(driver->getMotorSpeed (static_cast<MotorId>(i)), 0);
    }
    PtrTo<Motor<AdafruitMotorDriver<NullDevice> > > motor = new Motor<AdafruitMotorDriver<NullDevice> > (driver, MotorId::MOTOR_0);
    TEST_EQUALS(motor->getMotorId (), MotorId::MOTOR_0);
    TEST_EQUALS(driver->getMotorSpeed (MotorId::MOTOR_0), 0);

    // (runMotor) - motorId: MOTOR_1, speed: 1.0
    motor->run (1.0);
    TEST_EQUALS(driver->getMotorSpeed (MotorId::MOTOR_0), 1.0);

    // (runMotor) - motorId: MOTOR_1, speed: 1.5 - ensure the clamping works
    motor->run (1.5);
    TEST_EQUALS(driver->getMotorSpeed (MotorId::MOTOR_0), 1.0);
}

TEST_CASE(LiveTestMotor) {
    try {
        PtrTo<AdafruitMotorDriver<DeviceI2C> > driver = new AdafruitMotorDriver<DeviceI2C> ();
        PtrTo<Motor<AdafruitMotorDriver<DeviceI2C> > > motor = new Motor<AdafruitMotorDriver<DeviceI2C> > (driver, MotorId::MOTOR_2);
        Pause::milli (1000);
        motor->run (-0.5);
        Pause::milli (1000);
        motor->run (0.5);
        Pause::milli (1000);
        motor->run (-0.5);
        Pause::milli (1000);
        motor->run (0.5);
        Pause::milli (1000);
        motor->stop ();
    } catch (RuntimeError& runtimeError) {
        Log::exception (runtimeError);
    } catch (...) {
    }
    TEST_TRUE(true);
}
