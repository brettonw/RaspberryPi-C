#pragma once

#include "Bus.h"

// a general abstraction for a device on an I2C bus

const int DEVICE_I2C_ATVALUE_BUFFER_SIZE = 128;

MAKE_PTR_TO(DeviceI2C) {
    protected:
        struct Couplet {
            byte at;
            byte value;
            Couplet& operator () (byte _at, byte _value) {
                at = _at; value = _value; return *this;
            }
        };

        PtrToBus bus;
        uint address;
        uint length;
        Couplet couplets[DEVICE_I2C_ATVALUE_BUFFER_SIZE];


        // for testing purposes
        DeviceI2C () : address (0), length (0) {}

    public:
        DeviceI2C (uint _address, int _bus = -1) : bus ((_bus >= 0) ? Bus::getBusById(_bus) : Bus::getBusByIndex(-1 - _bus)), address(_address), length (0) {}

        ~DeviceI2C () {}

        DeviceI2C*  begin () {
            bus->begin (address);
            return this;
        }

        // reads are always immediate, after a flush
        byte read (byte at) {
            flush ();
            byte result = bus->readAt(at);
            Log::trace () << "DeviceI2C: " << "read (@" << hex (at) << ", got -> " << hex (result) << ")" << endl;
            return result;
        }

        DeviceI2C* read (byte at, byte* out) {
            *out = read (at);
            return this;
        }

        // writes are buffered
        DeviceI2C* write (byte at, byte value) {
            if (length < DEVICE_I2C_ATVALUE_BUFFER_SIZE) {
                couplets[length++] (at, value);
                Log::trace () << "DeviceI2C: " << "store couplet (@" << hex (at) << ", " << hex (value) << ")" << endl;
            } else {
                throw RuntimeError (Text ("DeviceI2C: ") << "out of write buffer.");
            }
            return this;
        }

        // finish any writes
        DeviceI2C* flush () {
            if (length > 0) {
                for (uint i = 0; i < length; ++i) {
                    Couplet& couplet = couplets[i];
                    bus->writeAt (couplet.at, couplet.value);
                }
                Log::trace () << "DeviceI2C: " << "flush " << length << " couplet" << ((length != 1) ? "s" : "") << endl;
                length = 0;
            }
            return this;
        }

        void end () {
            flush ();
            bus->end ();
        }
};

//------------------------------------------------------------------------------------------------------
