#pragma once

#include "Log.h"
#include "MotorId.h"

/**
* Motor
* brushed motors have relatively simple controllers that use two wires, and control the speed by
* switching the power on and off really fast using a modulated width pulse, a.k.a. PWM.
*/
template<typename DriverType>
class Motor : public ReferenceCountedObject {
    private:
        PtrTo<DriverType> driver;
        MotorId motorId;
        double speed;

    public:

    /**
    * contructor to use the default signal min and max values of 1ms and 2ms which are typical of
    * all servos, as near as I can tell. However, there is a lot of variance from servo to servo,
    * so this is really just a starting point for most servos.
    * @param servoController the controller to use for this servo
    * @param servoId         the id corresponding to the driver pins for this servo on the controller
    */
    Motor (PtrTo<DriverType> _driver, MotorId _motorId) : driver (_driver), motorId (_motorId) {
        stop ();
    }

    /**
    * @param speed - setting the speed at the controller (-1..1)
    *              -1 = full speed backward, 0 = stopped, 1 = full speed forward
    * @return
    */
    Motor* run (double _speed) {
        speed = min (max (_speed, -1.0), 1.0);
        Log::trace () << "Motor: " << "MOTOR_" << motorId << " @ " << speed << endl;
        driver->runMotor (motorId, speed);
        return this;
    }

    /**
     * @return
     */
    Motor* stop () {
        speed = 0;
        //Log::trace () << "Motor: " << "MOTOR_" << motorId << " @ STOP" << endl;
        driver->runMotor (motorId, speed);
        return this;
    }

    /**
     * @return
     */
    MotorId getMotorId () {
        return motorId;
    }

    /**
     * @return
     */
    double getSpeed () {
        return speed;
    }
};
