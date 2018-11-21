#pragma once

#include "RuntimeError.h"

// the default Broadcom (BCM) GPIO peripheral provides 54 GPIO pins, numbered 0..53
enum Pin {
    GPIO_00, GPIO_01, GPIO_02, GPIO_03, GPIO_04, GPIO_05, GPIO_06, GPIO_07, GPIO_08, GPIO_09,
    GPIO_10, GPIO_11, GPIO_12, GPIO_13, GPIO_14, GPIO_15, GPIO_16, GPIO_17, GPIO_18, GPIO_19,
    GPIO_20, GPIO_21, GPIO_22, GPIO_23, GPIO_24, GPIO_25, GPIO_26, GPIO_27, GPIO_28, GPIO_29,
    GPIO_30, GPIO_31, GPIO_32, GPIO_33, GPIO_34, GPIO_35, GPIO_36, GPIO_37, GPIO_38, GPIO_39,
    GPIO_40, GPIO_41, GPIO_42, GPIO_43, GPIO_44, GPIO_45, GPIO_46, GPIO_47, GPIO_48, GPIO_49,
    GPIO_50, GPIO_51, GPIO_52, GPIO_53
};
const int GPIO_PIN_COUNT = GPIO_53 + 1;

// the Raspberry Pi maps these pins to a 40-pin J-8 header, and only exposes some of the real GPIO
// pins. we provide a mapping for convenience. See https://pinout.xyz/ for reference
enum PiPin {
    RPI_01, RPI_02, RPI_03, RPI_04, RPI_05, RPI_06, RPI_07, RPI_08, RPI_09, RPI_10,
    RPI_11, RPI_12, RPI_13, RPI_14, RPI_15, RPI_16, RPI_17, RPI_18, RPI_19, RPI_20,
    RPI_21, RPI_22, RPI_23, RPI_24, RPI_25, RPI_26, RPI_27, RPI_28, RPI_29, RPI_30,
    RPI_31, RPI_32, RPI_33, RPI_34, RPI_35, RPI_36, RPI_37, RPI_38, RPI_39, RPI_40
};
const int RASPBERRY_PI_PIN_COUNT = RPI_40 + 1;
const int GPIO_XX = -1;

static inline Pin getPin (PiPin piPin) {
    extern int mapRpiPinToGpioPin[RASPBERRY_PI_PIN_COUNT];
    int gpioPin = mapRpiPinToGpioPin[piPin];
    if (gpioPin != GPIO_XX) {
        return static_cast<Pin> (gpioPin);
    }
    throw RuntimeError (Text("getPin: ") << "Raspberry Pi pin " << piPin << " does not correspond to a usable GPIO pin");
}

static inline PiPin getPiPin (Pin pin) {
    extern int mapRpiPinToGpioPin[RASPBERRY_PI_PIN_COUNT];
    for (int i = 0; i < RASPBERRY_PI_PIN_COUNT; ++i) {
        if (mapRpiPinToGpioPin[i] == pin) {
            return static_cast<PiPin> (i);
        }
    }
    throw RuntimeError (Text("getPiPin: ") << "GPIO pin " << pin << " does not correspond to a Raspberry Pi pin");
}

