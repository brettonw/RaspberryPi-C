#pragma once

#include "Motor.h"

// Stepper Motor
//
// stepper motors work by driving multiple coils in a sequence. this class drives bi-polar stepper
// motors, or motors that have two coils. The coils are activated using a pair of waveforms that are
// out of phase with each other. another way to think about the coil activations is to think of the
// power levels as the points on a unit circle, and the steps are progressing in cycles around the
// unit circle.
//
// this type of stepper is made using teeth internally that cause some number of detent positions
// for the motor. most motors will specify the "step angle", which is the angle associated with
// these detents in degrees (typical: 1.8 degrees)
template<typename DriverType>
class StepperMotor : public ReferenceCountedObject {
    private:
        // internal class for the values in a cycle
        struct CycleValue {
            double motorA;
            double motorB;

            CycleValue (double _motorA, double _motorB, bool _saturate) {
                motorA = _saturate ? saturate (_motorA) : _motorA;
                motorB = _saturate ? saturate (_motorB) : _motorB;
            }
        };

        enum { MINIMUM_CYCLE_DELAY = 10 };

        PtrTo<DriverType> driver;
        Text stepperType;
        MotorId motorIdA;
        MotorId motorIdB;
        double stepAngle;
        int stepsPerRevolution;
        int current;
        vector<CycleValue> cycle;

        StepperMotor (PtrTo<DriverType> _driver, Text _stepperType, MotorId _motorIdA, MotorId _motorIdB, double _stepAngle, int cycleLength, double startAngle, bool _saturate) :
            driver (_driver), stepperType (_stepperType), motorIdA(_motorIdA), motorIdB(_motorIdB),
            stepAngle (_stepAngle), stepsPerRevolution (int (round (360.0 / stepAngle))), current (0) {

            // build the cycle table - basically it is a representation of a list of 2d coordinates
            // taken to be positions on the unit circle, and traversed in angle order
            double cycleAngle = (M_PI * 2.0) / cycleLength;
            for (int i = 0; i < cycleLength; ++i) {
                double angle = startAngle + (cycleAngle * i);
                cycle.emplace_back (cos (angle), sin (angle), _saturate);
            }
    
            Log::info () << "StepperMotor:  " << getDescription () << ", with " << stepsPerRevolution << " steps per revolution" << endl;
    
            // and now... energize the coils at the start of the cycle
            step (0);
        }
    
        void step (int direction) {
            // add the direction for the step, and ensure the new index is in the valid region
            current += direction;
            int cycleSize = cycle.size ();
            do { current = (current + cycleSize) % cycleSize; } while (current < 0);
            Log::trace () << "StepperMotor: " << "current: " << current << ", A (" << cycle[current].motorA << "), B (" << cycle[current].motorB << ")" << endl;
            driver
                ->runMotor (motorIdA, cycle[current].motorA)
                ->runMotor (motorIdB, cycle[current].motorB);
        }
    

    public:
    // the most basic stepper traverses the unit circle, starting at 0 degrees and proceeds at 90
    // degree intervals in 4 steps. i find this method to be unreliable, often missing steps, so I
    // don't suggest using it. the next most basic case also proceeds at 90 degrees intervals in 4
    // steps, but starts at a 45 degree offset and saturates the control values. this way, both
    // coils are always fully activated, making the cycle robust.
    // @param stepAngle - the degrees per step from the motor specification
    // @param driver    - the motor driver, two motors are used to drive the stepper
    // @param motorIdA  - the first of the two motors, or "coils"
    // @param motorIdB  - the second of the two motors, or "coils"
    static PtrTo<StepperMotor<DriverType> > getFullStepper (PtrTo<DriverType> driver, MotorId motorIdA, MotorId motorIdB, double stepAngle) {
        return new StepperMotor (driver, "full", motorIdA, motorIdB, stepAngle, 4, M_PI / 4.0, true);
    }

    // a half-stepper starts at 0 degrees, proceeds at 45 degree intervals, and saturates the
    // control values. the result is more precise than a full step driver, but the torque varies
    // because the motor alternates between a single coil and both coils being activated.
    // @param stepAngle - the degrees per step from the motor specification
    // @param driver    - the motor driver, two motors are used to drive the stepper
    // @param motorIdA  - the first of the two motors, or "coils"
    // @param motorIdB  - the second of the two motors, or "coils"
    static PtrTo<StepperMotor<DriverType> > getHalfStepper (PtrTo<DriverType> driver, MotorId motorIdA, MotorId motorIdB, double stepAngle) {
        return new StepperMotor (driver, "half", motorIdA, motorIdB, stepAngle, 8, 0, true);
    }

    // a micro-stepper starts at 0 degrees and proceeds around the unit circle at sample points
    // according to the stepCount.
    //
    // @param stepAngle   - the degrees per step from the motor specification
    // @param driver      - the motor driver, two motors are used to drive the stepper
    // @param motorIdA    - the first of the two motors, or "coils"
    // @param motorIdB    - the second of the two motors, or "coils"
    // @param cycleLength - the number of internal steps per cycle. at higher counts, this can drive
    //                      the motor very precisely and smoothly, but the tradeoff is speed. useful
    //                      numbers start at 5 and go up.
    static PtrTo<StepperMotor<DriverType> > getMicroStepper (PtrTo<DriverType> driver, MotorId motorIdA, MotorId motorIdB, double stepAngle, int cycleLength) {
        return new StepperMotor (driver, "micro", motorIdA, motorIdB, stepAngle, cycleLength, 0, false);
    }

    // a micro-stepper starts at 0 degrees and proceeds around the unit circle at sample points
    // according to the desired angular resolution.
    // @param stepAngle   - the degrees per step from the motor specification
    // @param driver      - the motor driver, two motors are used to drive the stepper
    // @param motorIdA    - the first of the two motors, or "coils"
    // @param motorIdB    - the second of the two motors, or "coils"
    // @param resolution  - the desired accuracy of the motor.
    static PtrTo<StepperMotor<DriverType> > getMicroStepper (PtrTo<DriverType> driver, MotorId motorIdA, MotorId motorIdB, double resolution, double stepAngle) {
        // compute the closest approximation to the desired resolution
        int cycleLength = int (round ((stepAngle * 4.0) / resolution));
        return new StepperMotor (driver, "micro", motorIdA, motorIdB, stepAngle, cycleLength, 0, false);
    }

    StepperMotor<DriverType>* turn (double revolutions) {
        // do it as fast as possible
        return turn (revolutions, 0);
    }

    StepperMotor<DriverType>* turn (double revolutions, double time) {
        // XXX TODO this should be (optionally) threaded in the future

        // stepsPerRevolution is an artifical number based on the number of discrete positions of
        // the two energizing coils with the full step model - so we have to compensate if we use a
        // different cycle length. the number of full cycles through the stepsPerRevolution is given
        // by: stepsPerRevolution / 4. one is led to believe that all steppers have a
        // stepsPerRevolution that is evenly divisible by 4.
        int stepCount = int (round (abs (revolutions) * (((cycle.size () * stepsPerRevolution) / 4.0) + 1.0)));
        int direction = int (signum (revolutions));
        double halfway = stepCount / 2.0;

        // account for the delay time of going through this loop - assume 3us per iteration for
        // giggles - though empirical testing indicates the number is more in the 12-13 milliseconds
        // range. Maybe raspberry pi isn't the right platform to do this type of control? or maybe
        // the adafruit hat has too much communication overhead? or maybe Pi4J is based on a milli-
        // second time model, instead of a micro-second time model.
        double overheadTime = stepCount * 3;

        // we want to ramp the speed up and then back down to "soft start" the motor - giving us the
        // ability to overcome the motor's inherent internal inertia. for instance, where range =
        // 0.9, the varied speed will be (0.1 + (0.9 * x)) times the delay, where x will vary from 1
        // to 0 and back to 1 over the set of steps we will make. to keep the total time correct, we
        // compute the integral of the speed * time as the sum of the area of a box and a triangle.
        double speedVaryingRange = 0.9;
        double rangeTimeScale = 1.0 / ((1.0 - speedVaryingRange) + (0.5 * speedVaryingRange));

        // time is in seconds, scale it up to micro-seconds, scale by the rangeTimeScale, then
        // subtract the expected overhead. finally, divide by the number of steps we will take to
        // get the delay per step, rounded to the nearest integral microsecond.
        double microsecondsDelay = (rangeTimeScale * 1000000.0 * time) - overheadTime;
        int microsecondsDelayPerStep = max (int (round (microsecondsDelay / stepCount)), 0);
        Log::debug () << "StepperMotor: " << stepCount << " steps (direction: " << direction << ", delay: " << microsecondsDelayPerStep << ")" << endl;

        // loop over all the steps...
        //long timeSum = 0;
        for (int i = 0; i < stepCount; ++i) {
            //long startTime = System.nanoTime ();
            step (direction);
            double proportion = (1.0 - speedVaryingRange) + (speedVaryingRange * abs ((halfway - i) / halfway));
            int delay = int (round (microsecondsDelayPerStep * proportion));
            //log.debug ("delay: " + delay + "us");
            //timeSum += System.nanoTime () - startTime;
            Pause::micro (delay);
        }
        //Log::debug() << "StepperMotor: " << "Average overhead: " << (timeSum / stepCount) << "ns" << endl;
        return this;
    }

    StepperMotor<DriverType>* stop () {
        driver->runMotor (motorIdA, 0);
        driver->runMotor (motorIdB, 0);
        return this;
    }

    double getResolution () {
        return (stepAngle * 4.0) / cycle.size ();
    }

    Text getDescription () {
        return stepperType << "-step, cycle-length (per 4 steps): " << cycle.size () << ", resolution: " << getResolution () << " degrees/step";
    }
};
