/*
   RC_Car.ino
   Bluetooth Serial Connect — drive a rover with a joystick, slider, and toggle.

   Pairs with the "RC Car" premade board in the app (Templates -> Premade).
   It has a Joystick, Slider, and Toggle on channel 0.
*/

#include <BluetoothSerialConnect.h>

BluetoothSerialConnect serialConnect(Serial1, true);

const int HEADLIGHT_PIN = 13;
double speedLimit = 1.0;

void setup() {
  Serial.begin(9600);
  serialConnect.begin(9600);

  pinMode(HEADLIGHT_PIN, OUTPUT);
  Serial.println("RC car ready");
}

void loop() {
  serialConnect.readSerial();

  if (serialConnect.isSliderUpdated(0)) {
    speedLimit = serialConnect.getSlider(0) / 100.0;
    Serial.println(String("Speed limit: ") + speedLimit);
  }

  if (serialConnect.wasToggleUpdated(0)) {
    bool lightsOn = serialConnect.isToggleOn(0);

    digitalWrite(HEADLIGHT_PIN, lightsOn ? HIGH : LOW);
    serialConnect.sendAlert(lightsOn ? "Headlights on" : "Headlights off");
  }

  if (serialConnect.isJoystickUpdated(0)) {
    BluetoothSerialConnectJoystick stick = serialConnect.getJoystick(0);

    double steering = stick.getX();
    double throttle = stick.getY() * speedLimit;

    int leftMotor = constrain((int)((throttle + steering) * 255), -255, 255);
    int rightMotor = constrain((int)((throttle - steering) * 255), -255, 255);

    Serial.println(String("L: ") + leftMotor + "  R: " + rightMotor);
    // driveMotors(leftMotor, rightMotor);
  }
}
