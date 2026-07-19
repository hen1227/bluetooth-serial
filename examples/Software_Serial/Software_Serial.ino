/*
   Software_Serial.ino
   Bluetooth Serial Connect — talk to the BLE module over SoftwareSerial.

   Use this when your board has no Serial1 and you want the BLE module on
   spare pins instead of pins 0/1. Pairs with the Hello World board.
*/

#include <SoftwareSerial.h>
#include <BluetoothSerialConnect.h>

SoftwareSerial bluetoothModuleSerial(10, 11);
BluetoothSerialConnect serialConnect(bluetoothModuleSerial, true);

int pressCount = 0;

void setup() {
  Serial.begin(9600);
  bluetoothModuleSerial.begin(9600);
  serialConnect.begin(9600);

  Serial.println("Setup complete (SoftwareSerial)");
}

void loop() {
  serialConnect.readSerial();

  if (serialConnect.wasButtonPressed(0)) {
    pressCount++;
    serialConnect.sendAlert("Hello over SoftwareSerial!");
    serialConnect.setDisplay(String("Presses: ") + pressCount, 0);
  }

  if (serialConnect.hasMessage()) {
    serialConnect.setDisplay(serialConnect.getMessage(), 0);
  }
}
