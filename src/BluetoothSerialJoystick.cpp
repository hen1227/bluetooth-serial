/*
  BluetoothSerialJoystick.cpp - Easy Connections to Bluetooth
  Created by Henry Abrahamsen, March 3, 2023
  Released into the public domain.
*/

#include "BluetoothSerialJoystick.h"

BluetoothSerialJoystick::BluetoothSerialJoystick() {
    rotationDeg = 0;
    rotationRad = 0;
    magnitude = 0;
}

BluetoothSerialJoystick::BluetoothSerialJoystick(double rot, double mag) {
    rotationDeg = rot;
    rotationRad = rot * PI / 180.0;
    magnitude = mag;
}

void BluetoothSerialJoystick::updateValues(double rot, double mag) {
    rotationDeg = rot;
    rotationRad = rot * PI / 180.0;
    magnitude = mag;
}

double BluetoothSerialJoystick::getX() {
    return cos(rotationRad) * magnitude;
}

double BluetoothSerialJoystick::getY() {
    return sin(rotationRad) * magnitude;
}

double BluetoothSerialJoystick::getRotationDeg(double offset) {
    double completeAngle = 360.0;
    return fmod((rotationDeg - offset + completeAngle), completeAngle);
}

double BluetoothSerialJoystick::getRotationRad(double offset) {
    double completeAngle = (2.0 * PI);
    return fmod((rotationRad - offset + completeAngle), completeAngle);
}

double BluetoothSerialJoystick::getMagnitude() {
    return magnitude;
}