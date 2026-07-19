/*
   Hello_World.ino
   Bluetooth Serial Connect — the smallest complete example.

   Pairs with the "Hello World" example board in the app (Templates -> Examples).
   It has a Button on channel 0 and a Text Display on channel 0.
*/

#include <BluetoothSerialConnect.h>

BluetoothSerialConnect serialConnect(Serial1, true);

int pressCount = 0;

void setup() {
  Serial.begin(9600);
  serialConnect.begin(9600);

  Serial.println("Setup complete");
}

void loop() {
  serialConnect.readSerial();

  if (serialConnect.wasButtonPressed(0)) {
    pressCount++;

    Serial.println("Button B0 pressed!");
    serialConnect.sendAlert("Hello from the Arduino!");
    serialConnect.setDisplay(String("Presses: ") + pressCount, 0);
  }

  if (serialConnect.hasMessage()) {
    String text = serialConnect.getMessage();

    Serial.println(String("Console: ") + text);
    serialConnect.setDisplay(text, 0);
  }
}
