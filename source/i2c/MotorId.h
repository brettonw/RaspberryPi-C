#pragma once

enum class MotorId : byte {
    MOTOR_0, MOTOR_1, MOTOR_2, MOTOR_3
};

const uint MOTOR_COUNT = static_cast<byte>(MotorId::MOTOR_3) + 1;

static inline
ostream& operator << (ostream& stream, MotorId motorId) {
    return (stream << "MOTOR_" << static_cast<byte>(motorId));
}
