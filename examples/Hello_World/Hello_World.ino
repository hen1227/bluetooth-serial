/*
   Hello_World.ino
   Henry Abrahamsen
   8/12/23
   Simple code using the basic features of Bluetooth Serial
   Details available at https://docs.henhen1227.com/
*/

#include <BluetoothSerial.h>

#define bluetoothModuleSerial Serial1

// Create a BluetoothSerial object
// Serial port that the bluetooth module is connected
// Verbose mode: true
BluetoothSerial blueSerial(bluetoothModuleSerial, true);

void setup() {
  // Start communication with bluetooth device
  bluetoothModuleSerial.begin(9600);
  Serial.begin(9600);

  Serial.println("Setup Complete");
}


void loop() {
  blueSerial.readSerial();

  // If the button with id `B0`
  if (blueSerial.isButtonPressed(0)) {
    Serial.println("The button `B0` has been pressed!");

    // Send alert to the BluetoothSerial Connect App
    blueSerial.sendAlert("Hello from the Arduino");
  }

  // Check if the Joystick with id `J0` has updated
  if (blueSerial.isJoystickUpdated(0)) {
      
    // Get the Joystick object
    BluetoothSerialJoystick joystick = blueSerial.getJoystick(0);

    // Get the Joysticks rotation and magnitude
    String rotation = String(joystick.getRotationDeg(), 0);
    String magnitude = String(joystick.getMagnitude() * 100, 0); // As a percent %

    /// String(a, 0) is the double a with 0 trailing zeros as a String

    // Update the display inside the BluetoothSerial Connect App
    blueSerial.setDisplay("Joystick: " + rotation + "deg, " + magnitude + "%", 0);
  }
}
