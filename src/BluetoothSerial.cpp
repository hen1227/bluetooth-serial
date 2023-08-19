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
    clearBuffer();
    while (_serial->available()) {
        String str = readStringUntil(terminator, terminatingTimeout);  // Get letter from buffer

        if (_verbose) {
            Serial.println("BS received: `" + str + "`");  // Print out the command
        }

        if (str == "") return;

        if (str.startsWith("B")) {
            int newButton = str.substring(1).toInt();
            _buttonsList.insert(newButton);
        } else if (str.startsWith("J")) {
            String jValues = str.substring(1);
            double values[2];
            int id;

            int numValues = sscanf(jValues.c_str(), "%d:%lf,%lf", &id, &values[0], &values[1]);
            if (numValues == 3) {
                if (_joysticksList.count(id) > 0) {
                    _joysticksList[id].updateValues(values[0], values[1]);
                } else {
                    _joysticksList[id] = BluetoothSerialJoystick(values[0], values[1]);
                }

                _joysticksUpdatedList.insert(id);
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
    _buttonsList.clear();
    _joysticksUpdatedList.clear();
}

bool BluetoothSerial::isButtonPressed(int id) {
    return _buttonsList.count(id) > 0;
}

bool BluetoothSerial::isJoystickUpdated(int id) {
    return _joysticksUpdatedList.count(id) > 0;
}

BluetoothSerialJoystick BluetoothSerial::getJoystick(int id) {
    if (_joysticksList.count(id) > 0) {
        return _joysticksList[id];
    } else {
        return BluetoothSerialJoystick(0, 0);  // returns empty object
    }
}