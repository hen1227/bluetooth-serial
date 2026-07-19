#include <cassert>
#include <cmath>
#include <string>

#include "BluetoothSerialConnect.h"

HardwareSerial Serial;

static void useLegacy(BluetoothSerialConnect& bluetooth) {
    bluetooth.setProtocol(BluetoothSerialConnectLegacy);
}

static void testPartialButtonMessage() {
    HardwareSerial module;
    BluetoothSerialConnect bluetooth(module, false);
    useLegacy(bluetooth);

    module.pushIncoming("B0");
    bluetooth.readSerial();
    assert(!bluetooth.isButtonPressed(0));

    module.pushIncoming("\n");
    bluetooth.readSerial();
    assert(bluetooth.isButtonPressed(0));

    bluetooth.readSerial();
    assert(!bluetooth.isButtonPressed(0));
}

static void testJoystickMessage() {
    HardwareSerial module;
    BluetoothSerialConnect bluetooth(module, false);
    useLegacy(bluetooth);

    module.pushIncoming("J0:90,1.000\n");
    bluetooth.readSerial();

    assert(bluetooth.isJoystickUpdated(0));
    BluetoothSerialConnectJoystick joystick = bluetooth.getJoystick(0);
    assert(std::fabs(joystick.getX()) < 0.0001);
    assert(std::fabs(joystick.getY() - 1.0) < 0.0001);
}

static void testConsoleMessage() {
    HardwareSerial module;
    BluetoothSerialConnect bluetooth(module, false);
    useLegacy(bluetooth);

    module.pushIncoming("@start\n");
    bluetooth.readSerial();

    assert(bluetooth.hasMessage());
    assert(std::string(bluetooth.getMessage().c_str()) == "start");
    assert(!bluetooth.hasMessage());
}

static void testCustomProtocol() {
    HardwareSerial module;
    BluetoothSerialConnect bluetooth(module, false);
    useLegacy(bluetooth);

    assert(bluetooth.setButtonPrefix("BTN"));
    assert(bluetooth.setInputSuffix(";"));

    module.pushIncoming("BTN3;");
    bluetooth.readSerial();

    assert(bluetooth.isButtonPressed(3));
}

static void testTypeSpecificSuffixes() {
    HardwareSerial module;
    BluetoothSerialConnect bluetooth(module, false);
    useLegacy(bluetooth);

    assert(bluetooth.setButtonSuffix(";"));
    assert(bluetooth.setJoystickSuffix("|"));

    module.pushIncoming("J0:0,1.000");
    bluetooth.readSerial();
    assert(!bluetooth.isJoystickUpdated(0));

    module.pushIncoming("|");
    bluetooth.readSerial();
    assert(bluetooth.isJoystickUpdated(0));

    module.pushIncoming("B2;");
    bluetooth.readSerial();
    assert(bluetooth.isButtonPressed(2));
}

static void testOutputFormatting() {
    HardwareSerial module;
    BluetoothSerialConnect bluetooth(module, false);
    useLegacy(bluetooth);

    assert(bluetooth.setDisplayPrefix("D"));
    assert(bluetooth.setAlertPrefix("A"));
    assert(bluetooth.setOutputSuffix(";"));

    bluetooth.setDisplay("ready", 1);
    bluetooth.sendAlert("ok");

    assert(module.outgoing == "D1:ready;Aok;");
}

static void testModernIsDefault() {
    HardwareSerial module;
    BluetoothSerialConnect bluetooth(module, false);

    assert(bluetooth.protocol() == BluetoothSerialConnectModern);
    module.pushIncoming("B0:D\n");
    bluetooth.readSerial();
    assert(bluetooth.wasButtonPressed(0));
}

static void testModernButtonHeldAndReleased() {
    HardwareSerial module;
    BluetoothSerialConnect bluetooth(module, false);
    bluetooth.setProtocol(BluetoothSerialConnectModern);

    module.pushIncoming("B3:D\n");
    bluetooth.readSerial();
    assert(bluetooth.isButtonPressed(3));         // held
    assert(bluetooth.wasButtonPressed(3));        // down event is transient
    assert(!bluetooth.wasButtonRepeated(3));

    bluetooth.readSerial();                         // no new bytes
    assert(bluetooth.isButtonPressed(3));           // still held across reads
    assert(!bluetooth.wasButtonPressed(3));

    module.pushIncoming("B3:R\n");
    bluetooth.readSerial();
    assert(bluetooth.isButtonPressed(3));
    assert(bluetooth.wasButtonRepeated(3));

    module.pushIncoming("B3:U\n");
    bluetooth.readSerial();
    assert(!bluetooth.isButtonPressed(3));
    assert(bluetooth.wasButtonReleased(3));

    bluetooth.readSerial();
    assert(!bluetooth.wasButtonReleased(3));        // release flag is transient
}

static void testModernSliderToggleDpad() {
    HardwareSerial module;
    BluetoothSerialConnect bluetooth(module, false);
    bluetooth.setProtocol(BluetoothSerialConnectModern);

    module.pushIncoming("S2:42.5\n");
    bluetooth.readSerial();
    assert(bluetooth.isSliderUpdated(2));
    assert(std::fabs(bluetooth.getSlider(2) - 42.5) < 0.0001);

    module.pushIncoming("T0:1\n");
    bluetooth.readSerial();
    assert(bluetooth.isToggleOn(0));
    assert(bluetooth.wasToggleUpdated(0));
    bluetooth.readSerial();
    assert(!bluetooth.wasToggleUpdated(0));

    module.pushIncoming("T0:0\n");
    bluetooth.readSerial();
    assert(!bluetooth.isToggleOn(0));
    assert(bluetooth.wasToggleUpdated(0));

    module.pushIncoming("P0:L\n");
    bluetooth.readSerial();
    assert(bluetooth.isDpadPressed(0, BluetoothSerialConnectDpadLeft));
    assert(bluetooth.wasDpadPressed(0, BluetoothSerialConnectDpadLeft));
    bluetooth.readSerial();
    assert(!bluetooth.isDpadPressed(0, BluetoothSerialConnectDpadLeft));
    assert(!bluetooth.wasDpadPressed(0, BluetoothSerialConnectDpadLeft));

    module.pushIncoming("P0:U,D\n");
    bluetooth.readSerial();
    assert(bluetooth.isDpadPressed(0, BluetoothSerialConnectDpadUp));
    assert(bluetooth.wasDpadPressed(0, BluetoothSerialConnectDpadUp));

    module.pushIncoming("P0:U,R\n");
    bluetooth.readSerial();
    assert(bluetooth.wasDpadRepeated(0, BluetoothSerialConnectDpadUp));

    module.pushIncoming("P0:U,U\n");
    bluetooth.readSerial();
    assert(!bluetooth.isDpadPressed(0, BluetoothSerialConnectDpadUp));
    assert(bluetooth.wasDpadReleased(0, BluetoothSerialConnectDpadUp));
}

static void testModernConsoleEscaped() {
    HardwareSerial module;
    BluetoothSerialConnect bluetooth(module, false);
    bluetooth.setProtocol(BluetoothSerialConnectModern);

    module.pushIncoming("C:line1\\nline2\n");
    bluetooth.readSerial();
    assert(bluetooth.hasMessage());
    assert(std::string(bluetooth.getMessage().c_str()) == "line1\nline2");
}

static void testModernOutput() {
    HardwareSerial module;
    BluetoothSerialConnect bluetooth(module, false);
    bluetooth.setProtocol(BluetoothSerialConnectModern);

    bluetooth.setDisplay("a\nb", 1);   // newline escaped on the wire
    bluetooth.setLabel("hi", 3);
    bluetooth.setIndicator(true, 0);
    bluetooth.setGauge(73, 2);
    bluetooth.setGauge(12.345, 3, 1);
    bluetooth.sendAlert("ok");

    assert(module.outgoing == "D1:a\\nb\nL3:hi\nI0:1\nG2:73.00\nG3:12.3\nA:ok\n");
}

static void testModernRejectsMalformedFramesAndUnsupportedChannels() {
    HardwareSerial module;
    BluetoothSerialConnect bluetooth(module, false);

    const char* invalid[] = {
        "B0:Djunk\n",
        "B0:D,value\n",
        "B0:R,value\n",
        "B0:U,value\n",
        "J0:90,1junk\n",
        "S0:42junk\n",
        "T0:2\n",
        "P0:U,Djunk\n"
    };

    for (const char* message : invalid) {
        bluetooth.clearError();
        module.pushIncoming(message);
        bluetooth.readSerial();
        assert(bluetooth.lastError() != BluetoothSerialConnectNoError);
    }

    bluetooth.clearError();
    module.pushIncoming("J19:90,1\n");
    bluetooth.readSerial();
    assert(bluetooth.lastError() == BluetoothSerialConnectNoError);
    assert(bluetooth.isJoystickUpdated(BluetoothSerialConnectMaxChannel));

    bluetooth.clearError();
    module.pushIncoming("J20:90,1\n");
    bluetooth.readSerial();
    assert(bluetooth.lastError() == BluetoothSerialConnectInvalidChannel);
    assert(!bluetooth.isJoystickUpdated(19));
}

static void testSoftwareSerialStream() {
    // The generic Stream& constructor must accept a SoftwareSerial (a sibling of
    // HardwareSerial), read from it, and write back to the same stream.
    SoftwareSerial soft(10, 11);
    soft.begin(9600);
    BluetoothSerialConnect bluetooth(soft, false);   // selects the Stream& overload

    soft.pushIncoming("B0:D\n");
    bluetooth.readSerial();
    assert(bluetooth.wasButtonPressed(0));
    assert(bluetooth.isButtonPressed(0));

    bluetooth.sendAlert("ok");
    assert(soft.outgoing == "A:ok\n");

    soft.pushIncoming("S2:42.5\n");
    bluetooth.readSerial();
    assert(bluetooth.isSliderUpdated(2));
    assert(std::fabs(bluetooth.getSlider(2) - 42.5) < 0.0001);
}

int main() {
    testPartialButtonMessage();
    testJoystickMessage();
    testConsoleMessage();
    testCustomProtocol();
    testTypeSpecificSuffixes();
    testOutputFormatting();

    // Modern protocol
    testModernIsDefault();
    testModernButtonHeldAndReleased();
    testModernSliderToggleDpad();
    testModernConsoleEscaped();
    testModernOutput();
    testModernRejectsMalformedFramesAndUnsupportedChannels();

    // Stream / SoftwareSerial support
    testSoftwareSerialStream();
    return 0;
}
