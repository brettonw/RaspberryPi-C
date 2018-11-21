#include "Test.h"
#include "Pause.h"
#include "GPIO.h"

TEST_CASE(TestGPIO) {
    Log::Scope scope (Log::DEBUG);
    try {
        GPIO    gpio;

        const char* functions[] = {
            "I", "O", "F", "E", "A", "B", "C", "D"
        };

        GPIO::Function function;
        Log& log = Log::debug () << "TestGPIO: " << "Raspberry Pi pinout" << endl << endl;
        for (uint piPin = 0; piPin < RASPBERRY_PI_PIN_COUNT;) {
            log << ((piPin < 10) ? " " : "");
            try {
                Pin pin = getPin (static_cast<PiPin> (piPin));
                function = gpio.getFunction (pin);
                log << (piPin + 1) << " -> " << functions[function] << " | ";
            } catch (RuntimeError& runtimeError) {
                log << (piPin + 1) << " -> " << "*" << " | ";
            }
            ++piPin;
            try {
                Pin pin = getPin (static_cast<PiPin> (piPin));
                function = gpio.getFunction (pin);
                log << functions[function] << " <- " << (piPin + 1) << endl;
            } catch (RuntimeError& runtimeError) {
                log << "*" << " <- " << (piPin + 1) << endl;
            }
            ++piPin;
        }
        log << endl;

    } catch (RuntimeError& runtimeError) {
        Log::exception (runtimeError);
    } catch (...) {
    }
    TEST_TRUE(true);
}

TEST_CASE(TestBlinkPin6) {
    Log::Scope scope (Log::DEBUG);
    try {
        GPIO    gpio;
        gpio.setFunction (GPIO_06, GPIO::Function::OUTPUT);
        TEST_EQUALS(gpio.getFunction (GPIO_06), GPIO::Function::OUTPUT);
        gpio.clear (GPIO_06);
        TEST_EQUALS(gpio.get (GPIO_06), false);

        for (int i = 0; i < 10; ++i) {
            Log::debug () << "TestBlinkPin6: ON" << endl;
            gpio.toggle (GPIO_06);
            Pause::milli (500);
            Log::debug () << "TestBlinkPin6: OFF" << endl;
            gpio.toggle (GPIO_06);
            Pause::milli (500);
        }
    } catch (RuntimeError& runtimeError) {
        Log::exception (runtimeError);
    } catch (...) {
    }
    TEST_TRUE(true);
}

TEST_CASE(TestPinMappings) {
    //Log::Scope scope (Log::DEBUG);

    EXPECT_FAIL(getPin (RPI_01));      EXPECT_FAIL(getPin (RPI_02));
    TEST_EQUALS(getPin (RPI_03), GPIO_02); EXPECT_FAIL(getPin (RPI_04));
    TEST_EQUALS(getPin (RPI_05), GPIO_03); EXPECT_FAIL(getPin (RPI_06));
    TEST_EQUALS(getPin (RPI_07), GPIO_04); TEST_EQUALS(getPin (RPI_08), GPIO_14);
    EXPECT_FAIL(getPin (RPI_09));      TEST_EQUALS(getPin (RPI_10), GPIO_15);
    TEST_EQUALS(getPin (RPI_11), GPIO_17); TEST_EQUALS(getPin (RPI_12), GPIO_18);
    TEST_EQUALS(getPin (RPI_13), GPIO_27); EXPECT_FAIL(getPin (RPI_14));
    TEST_EQUALS(getPin (RPI_15), GPIO_22); TEST_EQUALS(getPin (RPI_16), GPIO_23);
    EXPECT_FAIL(getPin (RPI_17));      TEST_EQUALS(getPin (RPI_18), GPIO_24);
    TEST_EQUALS(getPin (RPI_19), GPIO_10); EXPECT_FAIL(getPin (RPI_20));
    TEST_EQUALS(getPin (RPI_21), GPIO_09); TEST_EQUALS(getPin (RPI_22), GPIO_25);
    TEST_EQUALS(getPin (RPI_23), GPIO_11); TEST_EQUALS(getPin (RPI_24), GPIO_08);
    EXPECT_FAIL(getPin (RPI_25));      TEST_EQUALS(getPin (RPI_26), GPIO_07);
    TEST_EQUALS(getPin (RPI_27), GPIO_00); TEST_EQUALS(getPin (RPI_28), GPIO_01);
    TEST_EQUALS(getPin (RPI_29), GPIO_05); EXPECT_FAIL(getPin (RPI_30));
    TEST_EQUALS(getPin (RPI_31), GPIO_06); TEST_EQUALS(getPin (RPI_32), GPIO_12);
    TEST_EQUALS(getPin (RPI_33), GPIO_13); EXPECT_FAIL(getPin (RPI_34));
    TEST_EQUALS(getPin (RPI_35), GPIO_19); TEST_EQUALS(getPin (RPI_36), GPIO_16);
    TEST_EQUALS(getPin (RPI_37), GPIO_26); TEST_EQUALS(getPin (RPI_38), GPIO_20);
    EXPECT_FAIL(getPin (RPI_39));      TEST_EQUALS(getPin (RPI_40), GPIO_21);


    TEST_EQUALS(getPiPin (GPIO_00), RPI_27);
    TEST_EQUALS(getPiPin (GPIO_01), RPI_28);
    TEST_EQUALS(getPiPin (GPIO_02), RPI_03);
    TEST_EQUALS(getPiPin (GPIO_03), RPI_05);
    TEST_EQUALS(getPiPin (GPIO_04), RPI_07);
    TEST_EQUALS(getPiPin (GPIO_05), RPI_29);
    TEST_EQUALS(getPiPin (GPIO_06), RPI_31);
    TEST_EQUALS(getPiPin (GPIO_07), RPI_26);
    TEST_EQUALS(getPiPin (GPIO_08), RPI_24);
    TEST_EQUALS(getPiPin (GPIO_09), RPI_21);
    TEST_EQUALS(getPiPin (GPIO_10), RPI_19);
    TEST_EQUALS(getPiPin (GPIO_11), RPI_23);
    TEST_EQUALS(getPiPin (GPIO_12), RPI_32);
    TEST_EQUALS(getPiPin (GPIO_13), RPI_33);
    TEST_EQUALS(getPiPin (GPIO_14), RPI_08);
    TEST_EQUALS(getPiPin (GPIO_15), RPI_10);
    TEST_EQUALS(getPiPin (GPIO_16), RPI_36);
    TEST_EQUALS(getPiPin (GPIO_17), RPI_11);
    TEST_EQUALS(getPiPin (GPIO_18), RPI_12);
    TEST_EQUALS(getPiPin (GPIO_19), RPI_35);
    TEST_EQUALS(getPiPin (GPIO_20), RPI_38);
    TEST_EQUALS(getPiPin (GPIO_21), RPI_40);
    TEST_EQUALS(getPiPin (GPIO_22), RPI_15);
    TEST_EQUALS(getPiPin (GPIO_23), RPI_16);
    TEST_EQUALS(getPiPin (GPIO_24), RPI_18);
    TEST_EQUALS(getPiPin (GPIO_25), RPI_22);
    TEST_EQUALS(getPiPin (GPIO_26), RPI_37);
    TEST_EQUALS(getPiPin (GPIO_27), RPI_13);

    EXPECT_FAIL(getPiPin (GPIO_28));
    EXPECT_FAIL(getPiPin (GPIO_29));
    EXPECT_FAIL(getPiPin (GPIO_30));
    EXPECT_FAIL(getPiPin (GPIO_31));
    EXPECT_FAIL(getPiPin (GPIO_32));
    EXPECT_FAIL(getPiPin (GPIO_33));
    EXPECT_FAIL(getPiPin (GPIO_34));
    EXPECT_FAIL(getPiPin (GPIO_35));
    EXPECT_FAIL(getPiPin (GPIO_36));
    EXPECT_FAIL(getPiPin (GPIO_37));
    EXPECT_FAIL(getPiPin (GPIO_38));
    EXPECT_FAIL(getPiPin (GPIO_39));
    EXPECT_FAIL(getPiPin (GPIO_40));
    EXPECT_FAIL(getPiPin (GPIO_41));
    EXPECT_FAIL(getPiPin (GPIO_42));
    EXPECT_FAIL(getPiPin (GPIO_43));
    EXPECT_FAIL(getPiPin (GPIO_44));
    EXPECT_FAIL(getPiPin (GPIO_45));
    EXPECT_FAIL(getPiPin (GPIO_46));
    EXPECT_FAIL(getPiPin (GPIO_47));
    EXPECT_FAIL(getPiPin (GPIO_48));
    EXPECT_FAIL(getPiPin (GPIO_49));
    EXPECT_FAIL(getPiPin (GPIO_50));
    EXPECT_FAIL(getPiPin (GPIO_51));
    EXPECT_FAIL(getPiPin (GPIO_52));
    EXPECT_FAIL(getPiPin (GPIO_53));
}
