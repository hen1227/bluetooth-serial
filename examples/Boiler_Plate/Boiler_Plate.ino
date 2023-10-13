/*
   Boiler_Plate.ino
   Henry Abrahamsen
   8/12/23
   Basic code "required" for all BluetoothSerial Projects
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

    // Do awesome stuff here!
    // 🍀 Good Luck 🍀!
}
