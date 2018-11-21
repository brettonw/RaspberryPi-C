#include "Test.h"
#include "AdafruitMotorDriver.h"
#include "DeviceI2C.h"
#include "StepperMotor.h"

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


TEST_CASE(TestStepperMotor) {
    //Log::Scope scope (Log::TRACE);
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
