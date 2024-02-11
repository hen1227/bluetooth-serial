/*
  BluetoothSerial.h - Easy Connections to Bluetooth
  Created by Henry Abrahamsen, March 3, 2023
  Released into the public domain.
*/

#ifndef BluetoothSerial_h
#define BluetoothSerial_h

#include <Arduino.h>
#include "BluetoothSerialJoystick.h"

#define MAX_BUTTONS 20 // Maximum number of buttons
#define MAX_JOYSTICKS 20 // Maximum number of joysticks


class BluetoothSerial {
private:
    HardwareSerial* _serial;
    bool _verbose;
    bool _buttonsList[MAX_BUTTONS];
    bool _joysticksUpdatedList[MAX_JOYSTICKS];
    BluetoothSerialJoystick _joysticksList[MAX_JOYSTICKS];

    /**
     * Reads input from the serial connection until the specified terminator character
     * is encountered or a timeout occurs.
     *
     * @param terminator The character to read up to.
     * @param timeout Duration (in milliseconds) before terminating the read operation.
     * @return The read string.
     */
    String readStringUntil(char terminator, int timeout);

public:
    /**
     * Constructor for initializing the BluetoothSerial object.
     *
     * @param serial Reference to a HardwareSerial object (e.g., Serial1).
     * @param verbose Indicates if debug messages should be printed to the Serial.
     */
    BluetoothSerial(HardwareSerial& serial, bool verbose = true);

    /**
     * Initialize the serial communication.
     *
     * Typically called within the Arduino setup() function.
     *
     * @param baudRate The baud rate for serial communication. Defaults to 9600.
     */
    void begin(int baudRate = 9600);

    /**
     * Reads and processes input from the Bluetooth serial connection.
     *
     * Typically called at the beginning of the Arduino loop() function.
     *
     * @param terminator Character that indicates the end of a message. Default is '\n'.
     * @param terminatingTimeout Duration (in milliseconds) to wait for the terminator. Default is 50ms.
     */
    void readSerial(char terminator = '\n', int terminatingTimeout = 50);

    /**
     * Sends a message a raw message to the connected iOS device.
     * The Bluetooth Serial won't process these
     *
     * @param message The content to be sent.
     */
    void writeSerial(String message);

    /**
     * Sends an alert message to the connected Bluetooth device. Longer messages may experience delays.
     *
     * @param message Alert content.
     */
    void sendAlert(String message);

    /**
     * Displays a message on the connected app.
     *
     * @param message The content to display on the app.
     * @param id Identifier for a specific display element in the app.
     */
    void setDisplay(String message, int id);

    /**
     * Clears the record of pressed buttons.
     *
     * After calling this method, all button states are set to "not pressed" until
     * they are read again by readSerial().
     * Typically called at the end of the Arduino loop() function.
     */
    void clearBuffer();

    /**
     * Checks if a specific button is currently pressed.
     *
     * @param id The identifier for the button as defined in the app.
     * @return True if the button is pressed, false otherwise.
     */
    bool isButtonPressed(int id);

    /**
     * Checks if a specific joystick has been updated since last call.
     *
     * @param id The identifier for the joystick as defined in the app.
     * @return True if the joystick is updated, false otherwise.
     */
    bool isJoystickUpdated(int id);


    /**
     * Gets string messages from app. Sent through the console section of the board.
     *
     * @return Message from app's console
     */
    String getMessage();

    /**
     * Retrieves the current state of a specific joystick.
     *
     * @param id The identifier for the joystick as defined in the app.
     * @return A BluetoothSerialJoystick object representing the joystick's current state.
     */
    BluetoothSerialJoystick getJoystick(int id);
};

#endif