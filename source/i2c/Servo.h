#pragma once

#include "Log.h"
#include "ServoId.h"

template<typename DriverType>
class Servo : public ReferenceCountedObject {
    private:
        PtrTo<DriverType> driver;
        ServoId servoId;
        double minPositionMs;
        double maxPositionMs;
        double position;

    public:

    /**
    * contructor to use the default signal min and max values of 1ms and 2ms which are typical of
    * all servos, as near as I can tell. However, there is a lot of variance from servo to servo,
    * so this is really just a starting point for most servos.
    * @param servoController the controller to use for this servo
    * @param servoId         the id corresponding to the driver pins for this servo on the controller
    */
    Servo (PtrTo<DriverType> _driver, ServoId _servoId, double _minPositionMs = 1.0, double _maxPositionMs = 2.0) : driver (_driver), servoId (_servoId), minPositionMs (_minPositionMs), maxPositionMs (_maxPositionMs) {
        setPosition (0);
    }

    /**
    * @param position - in the range -1..1
    * @return
    */
    Servo& setPosition (double _position) {
        position = min (max (_position, -1.0), 1.0);
        double pulseDurationMilliseconds = minPositionMs + ((maxPositionMs - minPositionMs) * (position + 1) * 0.5);
        Log::trace () << "Servo: " << servoId << "@" << pulseDurationMilliseconds << "ms" << endl;
        driver->setPulseDuration (servoId, pulseDurationMilliseconds);
        return *this;
    }

    /**
    * @return
    */
    double getPosition () {
        return position;
    }

    /**
    * @return
    */
    ServoId getServoId () {
        return servoId;
    }

};
