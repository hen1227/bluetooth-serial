/*
   Boiler_Plate.ino
   Bluetooth Serial Connect — minimum project scaffold.
*/

#include <BluetoothSerialConnect.h>

BluetoothSerialConnect serialConnect(Serial1, true);

void setup() {
  Serial.begin(9600);
  serialConnect.begin(9600);

  Serial.println("Setup complete");
}

void loop() {
  serialConnect.readSerial();

  // Read app controls and send app updates here.
}
