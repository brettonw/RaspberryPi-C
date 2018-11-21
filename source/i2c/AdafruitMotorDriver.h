#pragma once

#include "PCA9685.h"
#include "MotorId.h"

// DC and Stepper Motor "Hat" Driver
//
// https://learn.adafruit.com/adafruit-dc-and-stepper-motor-hat-for-raspberry-pi/overview
// https://cdn-shop.adafruit.com/datasheets/TB6612FNG_datasheet_en_20121101.pdf
//
// this "hat" is a combination 9685 16 Channel Pulse Width Modulation Controller (PWM) for LEDs, and
// 2 6612 H-bridge motor controllers driven off the modulated outputs. the "hat" supports four
// motors (a stepper motor is driven as if it were two motors)

const int ADAFRUIT_MOTOR_DRIVER_DEFAULT_ADDRESS = 0x60;

template<typename DeviceType>
class AdafruitMotorDriver : public PCA9685<DeviceType> {
    protected:
        // a helper class to capture the three pin values that we have to set through the PCA9685
        // for any given motor id
        struct MotorSpec {
            byte modulator;
            byte frontPin;
            byte backPin;

            void operator () (byte _modulator, byte _frontPin, byte _backPin) {
                modulator = _modulator;
                frontPin = _frontPin;
                backPin = _backPin;
            }
        };

        double speeds[MOTOR_COUNT];

        void stopAllMotors () {
            for (byte i = 0; i < MOTOR_COUNT; ++i) {
                runMotor (static_cast<MotorId>(i), 0.0);
            }
        }


    public:
        AdafruitMotorDriver (uint address = ADAFRUIT_MOTOR_DRIVER_DEFAULT_ADDRESS, uint requestedPulseFrequency = PCA9685_DEFAULT_PULSE_FREQUENCY, int bus = -1) : PCA9685<DeviceType> (address, requestedPulseFrequency, bus) {
            stopAllMotors ();
        }

        AdafruitMotorDriver (PtrTo<DeviceType> _device, uint requestedPulseFrequency = PCA9685_DEFAULT_PULSE_FREQUENCY) : PCA9685<DeviceType> (_device, requestedPulseFrequency) {
            stopAllMotors ();
        }

        /**
        * run a motor
        * @param motorId - which motor to run
        * @param speed - the speed to run it at in the range 0..1, 0 is stopped.
        */
        AdafruitMotorDriver<DeviceType>* runMotor (MotorId motorId, double speed) {
            MotorSpec motorSpec;
            switch (motorId) {
                case MotorId::MOTOR_0: motorSpec ( 8, 9, 10); break;
                case MotorId::MOTOR_1: motorSpec (13, 12, 11); break;
                case MotorId::MOTOR_2: motorSpec ( 2, 3, 4); break;
                case MotorId::MOTOR_3: motorSpec ( 7, 6, 5); break;
                // this line of code is to get around older gcc compilers (like 6), complaining
                // that motorSpec may not be initialized. I tried turning off the warning, but it
                // started a cascade of warning problems that were leading towards more complex
                // code, so I just bit the bullet - this will never happen.
                default:               motorSpec ( 0, 0, 0); break;
            }
            Log::trace () << "AdafruitMotorDriver: " << "run MOTOR_" << motorId << " ("
                          << "modulator [" << hex (motorSpec.modulator) << "], "
                          << "frontPin [" << hex (motorSpec.frontPin) << "], "
                          << "backPin [" << hex(motorSpec.backPin) << "]" << ") @ "
                          << speed << endl;

            if (speed < 0.0) {
                PCA9685<DeviceType>::setChannelOff (motorSpec.frontPin);
                PCA9685<DeviceType>::setChannelOn (motorSpec.backPin);
                PCA9685<DeviceType>::setChannelPulse (motorSpec.modulator, uint (-speed * PCA9685<DeviceType>::CHANNEL_HIGH));
            } else if (speed > 0.0) {
                PCA9685<DeviceType>::setChannelOn (motorSpec.frontPin);
                PCA9685<DeviceType>::setChannelOff (motorSpec.backPin);
                PCA9685<DeviceType>::setChannelPulse (motorSpec.modulator, uint (speed * PCA9685<DeviceType>::CHANNEL_HIGH));
            } else if (speed == 0.0) {
                PCA9685<DeviceType>::setChannelOff (motorSpec.frontPin);
                PCA9685<DeviceType>::setChannelOff (motorSpec.backPin);
                PCA9685<DeviceType>::setChannelOff (motorSpec.modulator);
            }

            // if we successfully got here, then capture the speed request
            speeds[static_cast<uint>(motorId)] = speed;

            return this;
        }

        double getMotorSpeed (MotorId motorId) {
            return speeds[static_cast<uint>(motorId)];
        }
};
