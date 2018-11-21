#pragma once

#include "Log.h"
#include "File.h"

// headers needed for open, close, and ioctl
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>

#ifndef __APPLE__
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#else
// shamelessly copied from a linux i2c.h and i2c-dev.h headers to facilitate compilation and unit
// testing on platforms other than linux, or that don't have an I2C bus. prefer the linux source if
// it is available

#define I2C_SLAVE               0x0703  // use this slave address
#define I2C_TENBIT              0x0704  // set to 0 for 7 bit addrs (pretty much everything we care about)
#define I2C_SMBUS               0x0720  // perform a SMBus operation

// SMBus read or write markers
#define I2C_SMBUS_READ          1
#define I2C_SMBUS_WRITE         0

// sizes of transfers for byte and byte data
#define I2C_SMBUS_BYTE          1
#define I2C_SMBUS_BYTE_DATA     2

#endif

// this is an abstraction on a System Management Bus (SMB) which we use for Inter-Integrated
// Circuit Bus (I2C) operations. Both are 2-wire bus protocols that are compatible with each other,
// but I2C can run at higher speeds and has simpler electric connection requirements. You can see in
// the linux constant naming that the same code is used for either bus protocol, though there is a
// slight lack of consistency in the naming protocols.

// we assume stuff works, so the general error handling strategy is to throw an exception if
// something fails.

const int BUS_INVALID = -1;
const int BUS_MAX_COUNT = 256;
#define BUS_FILE_PATH   "/dev/i2c-"

MAKE_PTR_TO(Bus) {
    private:
        uint id;
        Text filePath;
        int handle;

        // a mutex used to atomicize access to the bus
        pthread_mutex_t     mutex;
        pthread_mutexattr_t mutexAttribute;

        static map<int, PtrToBus> buses;

        struct BusControl {
            byte readWrite;
            byte command;
            uint size;
            byte* data;

            BusControl (byte _readWrite, byte _command, uint _size, byte*_data) :
                readWrite (_readWrite), command (_command), size (_size), data(_data) {}
        };

        void read (byte command, int size, byte* data) {
            BusControl control (I2C_SMBUS_READ, command, size, data);
            if (ioctl (handle, I2C_SMBUS, &control) != 0) {
                throw RuntimeError (Text("Bus: ") << "read error");
            }
        }

        void write (byte command, int size, byte* data) {
            BusControl control (I2C_SMBUS_WRITE, command, size, data);
            if (ioctl (handle, I2C_SMBUS, &control) != 0) {
                throw RuntimeError (Text("Bus: ") << "write error");
            }
        }

        Bus (uint _id, const Text& _filePath) : id (_id), filePath (_filePath), handle(BUS_INVALID) {
            // NOTE: constructing a bus doesn't "open" it - that is done lazily to avoid allocating
            // resources unnecessarily, but once it's opened it stays open until the program
            // terminates
            if (pthread_mutexattr_settype(&mutexAttribute, PTHREAD_MUTEX_RECURSIVE) == 0) {
                if (pthread_mutex_init (&mutex, &mutexAttribute) != 0) {
                    throw RuntimeError (Text ("Bus: ") << "can't create mutex");
                } else {
                    pthread_mutexattr_destroy(&mutexAttribute);
                }
            } else {
                throw RuntimeError (Text ("Bus: ") << "can't create mutex attribute [PTHREAD_MUTEX_RECURSIVE]");
            }

        }

        void close () {
            if (handle != BUS_INVALID) {
                ::close (handle);
                Log::info () << "Bus: " << "closed bus " << id << " (" << hex (handle) << ") on " << filePath << endl;
                handle = BUS_INVALID;
            }
        }

    public:
        static void identifyBuses () {
            // only do this once (or not, if there are no I2Cs on this system)
            if (buses.size () == 0) {
                for (uint i = 0; i < BUS_MAX_COUNT; ++i) {
                    auto smbusFilePath  = Text (BUS_FILE_PATH) << i;
                    if (File (smbusFilePath).getExists ()) {
                        buses[i] = new Bus (i, smbusFilePath);
                    }
                }
            }
        }

        // get bus by their file id (0..BUS_MAX_COUNT)
        static PtrToBus getBusById (uint id) {
            identifyBuses ();
            return buses.at (id);
        }

        // get bus by their index in the buses map, this might be the common case if you don't know
        // what bus to try to open - it will be consistent from run to run on a single platform, so
        // long as the file mappings of the buses don't change.
        static PtrToBus getBusByIndex (uint index) {
            identifyBuses ();
            if (index < buses.size()) {
                map<int, PtrToBus>::iterator iter = buses.begin();
               for (uint i = 0; i < index; ++i) {
                    ++iter;
                }
                return iter->second;
            } else {
                throw RuntimeError (Text("Bus: ") << "index out of range (" << index << ")");
            }
        }

        // destructor
        ~Bus () {
            if (handle >= 0) {
                close ();
            }
            pthread_mutexattr_destroy(&mutexAttribute);
            pthread_mutex_destroy(&mutex);
        }

        // start the read/write cycle on a bus
        Bus* begin (uint address) {
            // lock the mutex and increment the lock count
            if (pthread_mutex_lock(&mutex) != 0) {
                throw RuntimeError (Text("Bus: ") << "can't lock mutex");
            }

            // if the bus is not already open, open it
            if (handle == BUS_INVALID) {
                if ((handle = ::open (filePath.get (), O_RDWR)) != BUS_INVALID) {
                    Log::info () << "Bus: " << "opened bus " << id << " (" << hex (handle) << ") on " << filePath << endl;

                    // set it to use 7-bit addressing
                    if (ioctl (handle, I2C_TENBIT, 0) == 0) {
                        Log::debug () << "Bus: " << "    ...and set it to use 7-bit addressing" << endl;
                    } else {
                        close ();
                        throw RuntimeError (Text("Bus:") << "can't set 7-bit addressing (" << errno << ")");
                    }
                } else {
                    throw RuntimeError (Text("Bus: ") << "can't open bus on " << filePath);
                }
            }

            // set the slave address
            if (ioctl (handle, I2C_SLAVE, address) != 0) {
                throw RuntimeError (Text("Bus: ") << "can't set slave address");
            }

            // ready to do some work
            return this;
        }

        // finish working with the device
        void end () {
            if (pthread_mutex_unlock (&mutex) != 0) {
                throw RuntimeError (Text("Bus: ") << "can't unlock mutex");
            }
        }

        byte readByte () {
            byte data;
            read (0, I2C_SMBUS_BYTE, &data);
            return data;
        }

        Bus* writeByte (byte value) {
            write (value, I2C_SMBUS_BYTE, 0);
            return this;
        }

        byte readAt (byte at) {
            byte data[2];
            read (at, I2C_SMBUS_BYTE_DATA, &data[0]);
            return data[0];
        }

        Bus* writeAt (byte at, byte value) {
            byte data[2] = {value, 0x00};
            write (at, I2C_SMBUS_BYTE_DATA, &data[0]);
            return this;
        }
};
