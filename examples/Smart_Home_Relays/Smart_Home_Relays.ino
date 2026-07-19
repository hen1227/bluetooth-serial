/*
   Smart_Home_Relays.ino
   Bluetooth Serial Connect — toggles control three outputs.

   Pairs with the "Smart Home Relays" example board in the app (Templates -> Examples).
   It has Toggles and Indicators on channels 0, 1, and 2.
*/

#include <BluetoothSerialConnect.h>

BluetoothSerialConnect serialConnect(Serial1, true);

const int RELAY_PINS[3] = { 2, 3, 4 };

void setup() {
  Serial.begin(9600);
  serialConnect.begin(9600);

  for (int ch = 0; ch < 3; ch++) {
    pinMode(RELAY_PINS[ch], OUTPUT);
    digitalWrite(RELAY_PINS[ch], LOW);
    serialConnect.setIndicator(false, ch);
  }

  Serial.println("Relay panel ready");
}

void loop() {
  serialConnect.readSerial();

  for (int ch = 0; ch < 3; ch++) {
    if (serialConnect.wasToggleUpdated(ch)) {
      bool on = serialConnect.isToggleOn(ch);

      digitalWrite(RELAY_PINS[ch], on ? HIGH : LOW);
      serialConnect.setIndicator(on, ch);

      Serial.println(String("Relay ") + ch + (on ? " ON" : " OFF"));
    }
  }
}
