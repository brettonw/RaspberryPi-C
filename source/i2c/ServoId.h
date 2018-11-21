#pragma once

enum class ServoId : byte {
    SERVO_00, SERVO_01, SERVO_02, SERVO_03,
    SERVO_04, SERVO_05, SERVO_06, SERVO_07,
    SERVO_08, SERVO_09, SERVO_10, SERVO_11,
    SERVO_12, SERVO_13, SERVO_14, SERVO_15
};

const uint SERVO_COUNT = static_cast<byte>(ServoId::SERVO_15) + 1;

static inline
ostream& operator << (ostream& stream, ServoId servoId) {
    auto prevFill = stream.fill('0');
    auto prevWidth = stream.width(2);
    return (stream << "SERVO_" << static_cast<byte>(servoId) <<  setfill(prevFill) << setw(prevWidth));
}
