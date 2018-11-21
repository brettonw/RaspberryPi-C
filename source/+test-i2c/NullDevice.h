#pragma once

#include "Common.h"

MAKE_PTR_TO(NullDevice) {
    public:
        NullDevice () {}

        NullDevice (uint address, uint busNumber = 0) {}

        ~NullDevice () {}

        NullDevice*  begin () {
            return this;
        }

        // reads are always immediate, after a flush
        byte read (byte at) {
            return at;
        }

        NullDevice* read (byte at, byte* out) {
            *out = read (at);
            return this;
        }

        // writes are buffered
        NullDevice* write (byte at, byte value) {
            return this;
        }

        // finish any writes
        NullDevice* flush () {
            return this;
        }

        void end () {
        }

};

