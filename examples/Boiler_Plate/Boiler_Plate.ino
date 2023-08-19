/*
   Boiler_Plate.ino
   Henry Abrahamsen
   8/12/23
   Basic code "required" for all BluetoothSerial Projects
   Details available at https://docs.henhen1227.com/
*/

import BluetoothSerial;

#define bluetoothModuleSerial Serial1;

// Create a BluetoothSerial object
BluetoothSerial blueSerial;

void setup() {
    // Start communication with bluetooth device
    bluetoothModuleSerial.begin(9600);

    // Initialize the BluetoothSerial object
    // Baud rate: 9600
    // Verbose mode: false
    blueSerial = new BluetoothSerial(9600, false);

    Serial.println("Setup Complete");
}


void loop() {
    blueSerial.readSerial();

    // Do awesome stuff here!
    // 🍀 Good Luck 🍀!
}
