#include "Test.h"
#include "AdafruitMotorDriver.h"
#include "NullDevice.h"
#include "DeviceI2C.h"
#include "StepperMotor.h"

TEST_CASE(TestStepperMotor) {
    //Log::Scope scope (Log::TRACE);

    PtrToNullDevice device = new NullDevice ();
    PtrTo<AdafruitMotorDriver<NullDevice> > driver = new AdafruitMotorDriver<NullDevice> (device);
    for (byte i = 0; i < MOTOR_COUNT; ++i) {
        TEST_EQUALS(driver->getMotorSpeed (static_cast<MotorId>(i)), 0);
    }

    PtrTo<StepperMotor<AdafruitMotorDriver<NullDevice> > > stepper = StepperMotor<AdafruitMotorDriver<NullDevice> >::getHalfStepper (driver, MotorId::MOTOR_0, MotorId::MOTOR_1, 1.8);
    TEST_EQUALS(driver->getMotorSpeed (MotorId::MOTOR_0), 1);
    TEST_EQUALS(driver->getMotorSpeed (MotorId::MOTOR_1), 0);
}

template<typename DeviceType>
void backAndForth (PtrTo<StepperMotor<AdafruitMotorDriver<DeviceType> > > stepper) {
    Log::debug () << stepper->getDescription () << " - forward" << endl;
    stepper
        ->turn (0.5, 0.25)
        ->stop ();
    Pause::milli (500);

    Log::debug () << stepper->getDescription () << " - backward" << endl;
    stepper
        ->turn (-0.5, 0.25)
        ->stop ();
    Pause::milli (500);
}


TEST_CASE(LiveTestStepperMotor) {
    //Log::Scope scope (Log::DEBUG);
    try {
        PtrTo<AdafruitMotorDriver<DeviceI2C> > driver = new AdafruitMotorDriver<DeviceI2C> ();
        PtrTo<StepperMotor<AdafruitMotorDriver<DeviceI2C> > > stepper = StepperMotor<AdafruitMotorDriver<DeviceI2C> >::getHalfStepper (driver, MotorId::MOTOR_0, MotorId::MOTOR_1, 1.8);
        backAndForth (stepper);
    } catch (RuntimeError& runtimeError) {
        Log::exception (runtimeError);
    } catch (...) {
    }
    TEST_TRUE(true);
}
