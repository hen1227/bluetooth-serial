/*
   No_Serial1.ino
   Bluetooth Serial Connect — Hello World using the primary Serial port.

   Use this only when your board has no Serial1 and the BLE module is wired to
   pins 0 and 1. Disconnect the module while uploading.
*/

#include <BluetoothSerialConnect.h>

BluetoothSerialConnect serialConnect(Serial, false);

int pressCount = 0;

void setup() {
  serialConnect.begin(9600);
}

void loop() {
  serialConnect.readSerial();

  if (serialConnect.wasButtonPressed(0)) {
    pressCount++;
    serialConnect.sendAlert("Hello from the Arduino!");
    serialConnect.setDisplay(String("Presses: ") + pressCount, 0);
  }

  if (serialConnect.hasMessage()) {
    serialConnect.setDisplay(serialConnect.getMessage(), 0);
  }
}
