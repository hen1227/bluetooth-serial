/*
   Modern_Quickstart.ino
   Bluetooth Serial Connect — one compact sketch using every component type.

   Build a board with Button, Toggle, Slider, D-Pad, Joystick, Text Display,
   Label, Indicator, and Gauge on channel 0.
*/

#include <BluetoothSerialConnect.h>

BluetoothSerialConnect serialConnect(Serial1, true);

void setup() {
  Serial.begin(9600);
  serialConnect.begin(9600);

  Serial.println("Modern setup complete");
}

void loop() {
  serialConnect.readSerial();

  if (serialConnect.wasButtonPressed(0) || serialConnect.wasButtonRepeated(0)) {
    serialConnect.setIndicator(true, 0);
  }
  if (serialConnect.wasButtonReleased(0)) {
    serialConnect.setIndicator(false, 0);
  }

  if (serialConnect.wasToggleUpdated(0)) {
    serialConnect.setLabel(serialConnect.isToggleOn(0) ? "Motor: ON" : "Motor: OFF", 0);
  }

  if (serialConnect.isSliderUpdated(0)) {
    serialConnect.setGauge(serialConnect.getSlider(0), 0);
  }

  if (serialConnect.wasDpadPressed(0, BluetoothSerialConnectDpadUp)) serialConnect.setLabel("D-Pad Up", 0);
  if (serialConnect.wasDpadPressed(0, BluetoothSerialConnectDpadDown)) serialConnect.setLabel("D-Pad Down", 0);
  if (serialConnect.wasDpadPressed(0, BluetoothSerialConnectDpadLeft)) serialConnect.setLabel("D-Pad Left", 0);
  if (serialConnect.wasDpadPressed(0, BluetoothSerialConnectDpadRight)) serialConnect.setLabel("D-Pad Right", 0);

  if (serialConnect.isJoystickUpdated(0)) {
    BluetoothSerialConnectJoystick joystick = serialConnect.getJoystick(0);
    String rotation = String(joystick.getRotationDeg(), 0);
    String magnitude = String(joystick.getMagnitude() * 100, 0);

    serialConnect.setDisplay("Joy " + rotation + "deg " + magnitude + "%", 0);
  }

  if (serialConnect.hasMessage()) {
    serialConnect.sendAlert(serialConnect.getMessage());
  }
}
