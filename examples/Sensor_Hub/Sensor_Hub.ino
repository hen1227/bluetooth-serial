/*
   Sensor_Hub.ino
   Bluetooth Serial Connect — stream sensor readings to the app.

   Pairs with the "Sensor Hub" example board in the app (Templates -> Examples).
   It has Gauges on channels 0 and 1, plus a Label on channel 0.
*/

#include <BluetoothSerialConnect.h>

BluetoothSerialConnect serialConnect(Serial1, true);

const int TEMP_PIN = A0;
const int HUMIDITY_PIN = A1;

const unsigned long SEND_INTERVAL_MS = 200;
unsigned long lastSend = 0;

void setup() {
  Serial.begin(9600);
  serialConnect.begin(9600);

  serialConnect.setLabel("Warming up...", 0);
  Serial.println("Sensor hub ready");
}

void loop() {
  serialConnect.readSerial();

  unsigned long now = millis();
  if (now - lastSend < SEND_INTERVAL_MS) {
    return;
  }
  lastSend = now;

  double temperature = analogRead(TEMP_PIN) * (120.0 / 1023.0);
  double humidity = analogRead(HUMIDITY_PIN) * (100.0 / 1023.0);

  serialConnect.setGauge(temperature, 0, 1);
  serialConnect.setGauge(humidity, 1, 0);
  serialConnect.setLabel(temperature > 100 ? "Status: HOT" : "Status: OK", 0);
}
