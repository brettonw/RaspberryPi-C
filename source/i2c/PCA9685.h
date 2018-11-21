#pragma once

#include "Log.h"
#include "Pause.h"
#include "Text.h"

// values used for setting the pulse frequency, the default is 1ms per cycle
const double PCA9685_CLOCK_FREQUENCY = 25000000.0; // PCA9685 has a 25 MHz internal oscillator
const uint PCA9685_DEFAULT_PULSE_FREQUENCY = 1000;

// This is a software interface for the PCA9685. It is a 16-channel Pulse Width Modulator (PWM)
// Controller (designed to drive LEDs) with 12 bits of resolution, and controlled over the I2C bus.
// The 9685 is used in the Adafruit motor hat and the servo driver board
// https://cdn-shop.adafruit.com/datasheets/PCA9685.pdf
template<typename DeviceType>
class PCA9685 : public ReferenceCountedObject {
    protected:
        enum {
            // registers (https://cdn-shop.adafruit.com/datasheets/PCA9685.pdf - table 4)
            MODE1 = 0x00,
            MODE2 = 0x01,
            PRE_SCALE = 0xfe,

            // these registers are used as the base address of the full set of supported channels,
            // technically they are LED0_ON_L, LED0_ON_H, LED0_OFF_L, and LED0_OFF_H
            CHANNEL_BASE_ON = 0x06,
            CHANNEL_BASE_OFF = 0x08,

            // values used for offsetting the registers by channel, "ALL" is a special channel
            CHANNEL_OFFSET_MULTIPLIER = 4,
            CHANNEL_ALL = 0x3d,

            // the pulse width modulators (PWM) have 12-bit resolution
            CHANNEL_HIGH = 0x0fff, // 4095
            CHANNEL_FORCE = 0x1000, // 4096

            // bits (https://cdn-shop.adafruit.com/datasheets/PCA9685.pdf - mode 1, table 5)
            RESTART = 0x80,
            SLEEP = 0x10,
            ALLCALL = 0x01,

            // bits (https://cdn-shop.adafruit.com/datasheets/PCA9685.pdf - mode 2, table 6)
            OUTDRV = 0x04
        };

        // internal variables
        PtrTo<DeviceType> device;
        double pulseFrequency;

        // internal methods
        void init (uint requestedPulseFrequency) {
            // init, everything off
            setChannelPulse (CHANNEL_ALL, 0, 0);
            device
                ->begin ()
                ->write (MODE2, OUTDRV)
                ->write (MODE1, ALLCALL)
                ->end ();

            // the chip takes 500 microseconds to recover from changes to the control registers
            Pause::micro (500);

            // wake up
            device
                ->begin ()
                ->write (MODE1, device->read (MODE1) & ~SLEEP)
                ->end ();

            // the chip takes 500 microseconds to recover from turning off the SLEEP bit
            Pause::micro (500);

            Log::info () << "PCA9685: " << "ready to talk" << endl;

            // setup
            setPulseFrequency (requestedPulseFrequency);

        }

        // set a channel's pulse parameters - this applies per tick of the clock (set by the
        // pulse frequency).
        // @param channel - which channel of the PCM will be updated
        // @param on      - when to turn the pulse on within the tick, out of 4096, control
        //                  bit 4 (value 4096) will force the output on for the whole cycle
        // @param off     - when to turn the pulse off within the tick, out of 4096, off
        //                  should be greater than on, but control bit 4 (value 4096) will force
        //                  the output off for the whole
        void setChannelPulse (byte channel, u2 on, u2 off) {
            // (https://cdn-shop.adafruit.com/datasheets/PCA9685.pdf - Section 7.3.3)
            Log::debug () << "PCA9685: " << "setChannelPulse - CHANNEL(" << hex(channel) << ") ON(" << hex (on) << ") OFF(" << hex (off) << ")" << endl;
            auto channelOffset = channel * CHANNEL_OFFSET_MULTIPLIER;
            device
                ->begin ()
                ->write (CHANNEL_BASE_ON + channelOffset, on & 0x00ff)
                ->write (CHANNEL_BASE_ON + channelOffset + 1, (on >> 8) & 0x00ff)
                ->write (CHANNEL_BASE_OFF + channelOffset, off & 0x00ff)
                ->write (CHANNEL_BASE_OFF + channelOffset + 1, (off >> 8) & 0x00ff)
                ->end ();
        }

        // set a channel's pulse parameters - this applies per tick of the clock (set by the
        // pulse frequency).
        // @param channel - which channel of the PCM will be updated
        // @param width   - proportion of the pulse to be on, 0..4_095 (for 0..1)
        void setChannelPulse (byte channel, uint width) {
            switch (width) {
                case 0:
                    setChannelPulse (channel, 0, CHANNEL_FORCE);
                    break;
                case CHANNEL_HIGH:
                    setChannelPulse (channel, CHANNEL_FORCE, 0);
                    break;
                default:
                    setChannelPulse (channel, 0, width);
                    break;
            }
        }

        void setChannelOn (byte channel) {
            setChannelPulse (channel, CHANNEL_FORCE, 0);
        }

        void setChannelOff (byte channel) {
            setChannelPulse (channel, 0, CHANNEL_FORCE);
        }

        void setChannelPulseMs (byte channel, double milliseconds) {
            uint width = (uint) round ((CHANNEL_HIGH * milliseconds * pulseFrequency) / 1.0e3);
            setChannelPulse (channel, width);
        }

    public:

        PCA9685 (uint address, uint requestedPulseFrequency = PCA9685_DEFAULT_PULSE_FREQUENCY, int bus = -1) : device (new DeviceType (address, bus)){
            init (requestedPulseFrequency);
        }

        PCA9685 (PtrTo<DeviceType> _device, uint requestedPulseFrequency = PCA9685_DEFAULT_PULSE_FREQUENCY) : device (_device){
            init (requestedPulseFrequency);
        }

         // Set the frequency of pulses across the whole controller - each channel has 12-bits
         // of resolution (4,096 division) for setting the pulse duration within the cycle
         // @param requestedPulseFrequency requested number of pulses per second for the whole board,
         //                                value in Hertz (Hz). The code tries to accommodate the request
         //                                as best as it can, to be *at least* as frequent as requested.
        void setPulseFrequency (uint requestedPulseFrequency, double clockFrequency = PCA9685_CLOCK_FREQUENCY) {
            // (https://cdn-shop.adafruit.com/datasheets/PCA9685.pdf - Section 7.3.5)
            const double CHANNEL_RESOLUTION = 4096.0;   // 12-bit precision
            byte preScale = byte (round (clockFrequency / (CHANNEL_RESOLUTION * requestedPulseFrequency))) - 1;
            Log::debug () << "PCA9685: " << "pre-scale (" << hex (preScale) << ")" << endl;
            const byte MIN_PRE_SCALE = 0x03, MAX_PRE_SCALE = 0xFF;
            preScale = min (max (MIN_PRE_SCALE, preScale), MAX_PRE_SCALE);
            Log::info () << "PCA9685: " << "requested @" << requestedPulseFrequency << " Hz";

            // compute the *actual* pulse frequency by inverting the equation
            pulseFrequency = clockFrequency / (CHANNEL_RESOLUTION * (preScale + 1));
            Log::info () << ", " << "actual @" << pulseFrequency << "Hz" << endl;

            // PRE_SCALE can only be set when the SLEEP bit of the MODE1 register is set to logic 1.
            byte oldMode = 0x00;
            device
                ->begin ()
                ->read (MODE1, &oldMode)
                ->write (MODE1, (oldMode & 0x7f) | SLEEP)
                ->write (PRE_SCALE, preScale)
                ->write (MODE1, oldMode)
                ->flush ();

            // SLEEP bit must be 0 for at least 500us before 1 is written into the RESTART bit.
            Pause::micro (500);

            // restart
            device
                ->write (MODE1, oldMode | RESTART)
                ->end ();
        }
};

