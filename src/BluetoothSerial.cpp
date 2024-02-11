/*
  BluetoothSerial.cpp - Easy Connections to Bluetooth
  Created by Henry Abrahamsen, March 3, 2023
  Released into the public domain.
*/

#include "BluetoothSerialJoystick.h"
#include "BluetoothSerial.h"

BluetoothSerial::BluetoothSerial(HardwareSerial& serial, bool verbose /*= true*/) {
    _serial = &serial;
    _verbose = verbose;
}

void BluetoothSerial::begin(int baudRate /*= 9600*/) {
    _serial->begin(baudRate);
    if (_verbose) {
        Serial.println("BluetoothSerial Begun");
    }
}

void BluetoothSerial::readSerial(char terminator /* = '\n' */, int terminatingTimeout /* = 50 */) {
    clearBuffer(); // Resets the tracking arrays
    while (_serial->available()) {
        String str = readStringUntil(terminator, terminatingTimeout); // Get command from buffer

        if (_verbose) {
            Serial.println("BS received: `" + str + "`"); // Print out the command
        }

        if (str == "") return;

        if (str.startsWith("B")) {
            int newButton = str.substring(1).toInt();
            if (newButton >= 0 && newButton < MAX_BUTTONS) {
                _buttonsList[newButton] = true; // Mark button as pressed
            }
        } else if (str.startsWith("J")) {
            String jValues = str.substring(1);
            double values[2];
            int id;

            int numValues = sscanf(jValues.c_str(), "%d:%lf,%lf", &id, &values[0], &values[1]);
            if (numValues == 3 && id >= 0 && id < MAX_JOYSTICKS) {
                _joysticksList[id].updateValues(values[0], values[1]);
                _joysticksUpdatedList[id] = true; // Mark joystick as updated
            } else {
                if (_verbose) {
                    Serial.println("Error with BluetoothSerial Library!");
                }
            }
        }
    }
}

// TODO: Make this possible asynchronously
// Currently it relies on a timeout, which
// may not be ideal for all projects.
String BluetoothSerial::readStringUntil(char terminator, int timeout) {
    String str;
    unsigned long startTime = millis();  // Get current time in milliseconds
    unsigned long timeoutLong = (unsigned long) timeout;
    while (millis() - startTime < timeoutLong) {  // Loop until timeout occurs
        while (_serial->available() > 0) {
            char c = _serial->read();
            if (c == terminator) {
                return str;
            }
            str += c;
        }
    }
    return "";
}

void BluetoothSerial::writeSerial(String message) {
    _serial->write(message.c_str());
}

void BluetoothSerial::sendAlert(String message) {
    String alertMessage = "@" + message + "\n";
    writeSerial(alertMessage);
}

void BluetoothSerial::setDisplay(String message, int id) {
    String displayMessage = "#" + String(id) + ":" + message + "\n";
    writeSerial(displayMessage);
}

void BluetoothSerial::clearBuffer() {
    for (int i = 0; i < MAX_BUTTONS; i++) {
        _buttonsList[i] = false;
    }
    for (int i = 0; i < MAX_JOYSTICKS; i++) {
        _joysticksUpdatedList[i] = false;
    }
}

bool BluetoothSerial::isButtonPressed(int id) {
    if (id >= 0 && id < MAX_BUTTONS) {
        return _buttonsList[id];
    }
    return false;
}

bool BluetoothSerial::isJoystickUpdated(int id) {
    if (id >= 0 && id < MAX_JOYSTICKS) {
        return _joysticksUpdatedList[id];
    }
    return false;
}

BluetoothSerialJoystick BluetoothSerial::getJoystick(int id) {
    if (id >= 0 && id < MAX_JOYSTICKS) {
        return _joysticksList[id];
    } else {
        return BluetoothSerialJoystick(0, 0);  // returns empty object
    }
}