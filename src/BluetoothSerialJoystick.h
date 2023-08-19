/*
  BluetoothSerialJoystick.h - Easy Connections to Bluetooth
  Created by Henry Abrahamsen, March 3, 2023
  Released into the public domain.
*/

#ifndef BLUETOOTH_SERIAL_JOYSTICK_H
#define BLUETOOTH_SERIAL_JOYSTICK_H

#include <Arduino.h>

class BluetoothSerialJoystick {
private:
    double rotationDeg;  // Rotation of the joystick in degrees
    double rotationRad;  // Rotation of the joystick in radians
    double magnitude;    // Distance of the joystick from the center

public:
    /**
     * Default constructor initializing joystick values to zero.
     */
    BluetoothSerialJoystick();

    /**
     * Parameterized constructor for joystick values.
     *
     * @param rot Initial rotation value in degrees.
     * @param mag Initial magnitude or distance from center.
     */
    BluetoothSerialJoystick(double rot, double mag);

    /**
     * Update joystick rotation and magnitude values.
     *
     * @param rot New rotation value in degrees.
     * @param mag New magnitude or distance from center.
     */
    void updateValues(double rot, double mag);

    /**
     * Retrieve the joystick's horizontal (X-axis) position.
     *
     * @returns the `x` position of the joystick, ranging from -1 to 1.
     */
    double getX();

    /**
     * Retrieve the joystick's vertical (Y-axis) position.
     *
     * @returns the `y` position of the joystick, ranging from -1 to 1.
     */
    double getY();

    /**
     * Retrieve the joystick's rotation in degrees.
     *
     * @param offset Optional degree offset to adjust the returned value.
     * @returns The rotation of the joystick in degrees, adjusted by the offset.
     */
    double getRotationDeg(double offset = 0);

    /**
     * Retrieve the joystick's rotation in radians.
     *
     * @param offset Optional radian offset to adjust the returned value.
     * @returns The rotation of the joystick in radians, adjusted by the offset.
     */
    double getRotationRad(double offset = 0);

    /**
     * Retrieve the joystick's distance from the center.
     *
     * @returns The magnitude of the joystick's position, with a maximum of 1.00.
     */
    double getMagnitude();
};

#endif
