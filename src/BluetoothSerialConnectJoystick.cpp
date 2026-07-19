/*
  BluetoothSerialConnectJoystick.cpp - Bluetooth Serial Connect joystick value
  Created by Henry Abrahamsen, March 3, 2023
  Released into the public domain.
*/

#include "BluetoothSerialConnectJoystick.h"

BluetoothSerialConnectJoystick::BluetoothSerialConnectJoystick() {
    rotationDeg = 0;
    rotationRad = 0;
    magnitude = 0;
}

BluetoothSerialConnectJoystick::BluetoothSerialConnectJoystick(double rot, double mag) {
    rotationDeg = rot;
    rotationRad = rot * PI / 180.0;
    magnitude = mag;
}

void BluetoothSerialConnectJoystick::updateValues(double rot, double mag) {
    rotationDeg = rot;
    rotationRad = rot * PI / 180.0;
    magnitude = mag;
}

double BluetoothSerialConnectJoystick::getX() {
    return cos(rotationRad) * magnitude;
}

double BluetoothSerialConnectJoystick::getY() {
    return sin(rotationRad) * magnitude;
}

double BluetoothSerialConnectJoystick::getRotationDeg(double offset) {
    double completeAngle = 360.0;
    return fmod((rotationDeg - offset + completeAngle), completeAngle);
}

double BluetoothSerialConnectJoystick::getRotationRad(double offset) {
    double completeAngle = (2.0 * PI);
    return fmod((rotationRad - offset + completeAngle), completeAngle);
}

double BluetoothSerialConnectJoystick::getMagnitude() {
    return magnitude;
}
