/*
   Menu_Remote.ino
   Bluetooth Serial Connect — navigate a menu with a D-Pad and two buttons.

   Pairs with the "Menu Remote" premade board in the app (Templates -> Premade).
   It has a D-Pad on channel 0 and Buttons on channels 0 and 1.
*/

#include <BluetoothSerialConnect.h>

BluetoothSerialConnect serialConnect(Serial1, true);

const char* MENU[] = { "Play", "Settings", "About", "Power" };
const int MENU_COUNT = 4;
int selection = 0;

void printMenu();

void setup() {
  Serial.begin(9600);
  serialConnect.begin(9600);

  Serial.println("Menu remote ready");
  printMenu();
}

void loop() {
  serialConnect.readSerial();

  // D-Pad ch 0: move the highlight.
  if (serialConnect.wasDpadPressed(0, BluetoothSerialConnectDpadUp)) {
    selection = (selection - 1 + MENU_COUNT) % MENU_COUNT;
    printMenu();
  }
  if (serialConnect.wasDpadPressed(0, BluetoothSerialConnectDpadDown)) {
    selection = (selection + 1) % MENU_COUNT;
    printMenu();
  }
  if (serialConnect.wasDpadPressed(0, BluetoothSerialConnectDpadLeft) ||
      serialConnect.wasDpadPressed(0, BluetoothSerialConnectDpadRight)) {
    Serial.println("(adjust value)");
    serialConnect.sendAlert(String("Adjust ") + MENU[selection]);
  }

  if (serialConnect.wasButtonPressed(0)) {
    Serial.println(String("Selected: ") + MENU[selection]);
    serialConnect.sendAlert(String("Opening ") + MENU[selection]);
  }
  if (serialConnect.wasButtonPressed(1)) {
    Serial.println("Back");
    serialConnect.sendAlert("Back");
  }
}

void printMenu() {
  Serial.print("Menu -> ");
  Serial.println(MENU[selection]);
  serialConnect.sendAlert(MENU[selection]);
}
