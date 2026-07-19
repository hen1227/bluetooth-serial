# Bluetooth Serial Connect

Arduino library for the **Bluetooth Serial Connect** iOS app. Build a custom touch
controller in the app — buttons, joysticks, sliders, toggles, D-Pads, displays,
labels, indicators, and gauges — and read/drive it from a few lines of sketch code
over a BLE serial module (HM-10 or compatible).

The app and this library are two halves of one system, joined by a small
newline-terminated ASCII protocol. This library speaks the **modern typed protocol**
by default, matching the current app — no configuration needed.

- App Store: https://apps.apple.com/us/app/bluetooth-serial-connect/id6449396821
- Documentation: https://bluetooth.henhen1227.com/

## Install

- **Arduino Library Manager:** search for `Bluetooth Serial Connect`.
- **From source:** *Sketch → Include Library → Add .ZIP Library*, or copy this folder
  into your Arduino `libraries/` directory.

### Migrating from 1.x

Version 2 renames the Arduino API to **Bluetooth Serial Connect** so it no longer
collides with Espressif's unrelated `BluetoothSerial.h` library for ESP32 Classic
Bluetooth. Update `#include <BluetoothSerial.h>` to
`#include <BluetoothSerialConnect.h>` and rename public `BluetoothSerial…` types and
constants to `BluetoothSerialConnect…`. For example, `BluetoothSerial` becomes
`BluetoothSerialConnect`, and `BluetoothSerialModern` becomes
`BluetoothSerialConnectModern`.

The v2 package intentionally does not provide a `BluetoothSerial.h` compatibility
header because doing so would recreate the ESP32 header collision.

## Wiring

A BLE serial module bridges iOS (Bluetooth Low Energy) and your microcontroller (UART):

| Module | Microcontroller |
|---|---|
| TX | RX |
| RX | TX |
| GND | GND |
| VCC | 3.3V / 5V (check your module) |

Use a **BLE** module (HM-10 and similar). Classic Bluetooth modules (HC-05 / HC-06)
are **not** compatible with iOS. On boards with a spare hardware UART (Mega, Leonardo,
Teensy, ESP32...) wire to `Serial1`. The main examples use this setup because it
keeps the USB `Serial` monitor free for debug logs.

### Hardware or software serial

`BluetoothSerialConnect` accepts any serial-like `Stream`, so it works with either:

```cpp
// Recommended when your board has a spare hardware UART.
BluetoothSerialConnect serialConnect(Serial1, true);
```

```cpp
// Boards with no Serial1 can use the primary Serial pins 0/1.
// Disable verbose logging because Serial is also the BLE link.
BluetoothSerialConnect serialConnect(Serial, false);
```

```cpp
// Or use SoftwareSerial on spare pins.
#include <SoftwareSerial.h>
SoftwareSerial mySerial(10, 11);        // RX, TX
BluetoothSerialConnect serialConnect(mySerial, true);
// ...in setup(): mySerial.begin(9600);
```

The library never includes `SoftwareSerial.h`, so it stays optional — only sketches
that need it pull it in. See `No_Serial1` for primary `Serial` pins and
`Software_Serial` for SoftwareSerial pins. With a `SoftwareSerial`, call `begin()` on
that port yourself; `BluetoothSerialConnect::begin()` only opens hardware UARTs.

### Arduino Uno memory

The Uno has only 2 KB of SRAM. The intentionally small `No_Serial1` and
`Software_Serial` examples fit, but can use roughly 85% of that memory and leave little
headroom for extra strings, buffers, sensors, or application state. For larger projects,
prefer a Mega, Leonardo, Teensy, ESP32, or another board with more SRAM, and keep Uno
sketches minimal.

## Quick start

```cpp
#include <BluetoothSerialConnect.h>

BluetoothSerialConnect serialConnect(Serial1, true);

void setup() {
  Serial.begin(9600);
  serialConnect.begin(9600);
}

void loop() {
  serialConnect.readSerial();                 // refresh control state every loop

  if (serialConnect.wasButtonPressed(0)) {    // button B0 was tapped
    serialConnect.setDisplay("Pressed", 0);   // update text display D0
    serialConnect.sendAlert("Hello!");        // pop up an alert in the app
  }
}
```

## Examples → boards

Each example pairs with a board you can open in the app under **Templates**. Open the
matching board, upload the sketch, connect, and the components light up.

| Example | App board | Components it demonstrates |
|---|---|---|
| `Boiler_Plate` | (any) | The minimum scaffold to copy from. |
| `Hello_World` | Hello World *(Example)* | Button, Text Display, alert, console. |
| `Smart_Home_Relays` | Smart Home Relays *(Example)* | Toggles → Indicator LEDs. |
| `Sensor_Hub` | Sensor Hub *(Example)* | Gauges + Label (device → app telemetry). |
| `RC_Car` | RC Car *(Premade)* | Joystick + Slider + Toggle. |
| `Menu_Remote` | Menu Remote *(Premade)* | D-Pad + Buttons. |
| `Modern_Quickstart` | (build your own) | One of every API call in a single file. |
| `No_Serial1` | Hello World *(Example)* | Hello World on boards using primary `Serial` pins. |
| `Software_Serial` | Hello World *(Example)* | Hello World, but over SoftwareSerial pins. |

## Components & the calls that drive them

Components are addressed by a **channel** (0–19) that is namespaced per type — a Button
and a Joystick can both be channel 0. The channel is set on each component in the app's
inspector; match it in your sketch.

### App → device (controls you read)

| Component | Read it with |
|---|---|
| **Button** | `wasButtonPressed(ch)`, `wasButtonReleased(ch)`, `wasButtonRepeated(ch)`, `isButtonPressed(ch)` (held). |
| **Joystick** | `isJoystickUpdated(ch)` then `getJoystick(ch)` → `getX()`, `getY()`, `getRotationDeg()`, `getRotationRad()`, `getMagnitude()`. |
| **Slider** | `isSliderUpdated(ch)` then `getSlider(ch)`. |
| **Toggle** | `isToggleOn(ch)`, `wasToggleUpdated(ch)`. |
| **D-Pad** | `wasDpadPressed(ch, dir)`, `wasDpadReleased(ch, dir)`, `wasDpadRepeated(ch, dir)`, `isDpadPressed(ch, dir)`. `dir` is `BluetoothSerialConnectDpadUp/Down/Left/Right` (the app's D-Pad sends these four; diagonals and `…DpadCenter` exist for custom layouts). |
| **Console** | `hasMessage()` then `getMessage()` (text typed in the app console). |

### Device → app (widgets you update)

| Component | Update it with |
|---|---|
| **Text Display** | `setDisplay(text, ch)` — LCD-styled text. |
| **Label** | `setLabel(text, ch)` — plain text. |
| **Indicator** | `setIndicator(on, ch)` — on/off LED. |
| **Gauge** | `setGauge(value, ch)` or `setGauge(value, ch, decimals)` — fills within the gauge's configured range. |
| **Alert** | `sendAlert(text)` — transient pop-up (no channel). |

Display/label/console/alert text is escaped on the wire, so it may contain newlines.

## Wire protocol

Newline-terminated ASCII. One verb per message.

**App → device:** `B{ch}:D|U|R` (button down/up/repeat) · `J{ch}:{rot},{mag}`
(joystick) · `S{ch}:{value}` (slider) · `T{ch}:{0|1}` (toggle) ·
`P{ch}:{dir},{D|U|R}` (D-Pad down/up/repeat; legacy direction-only frames are also accepted) ·
`C:{text}` (console).

**Device → app:** `D{ch}:{text}` (display) · `L{ch}:{text}` (label) · `I{ch}:{0|1}`
(indicator) · `G{ch}:{value}` (gauge) · `A:{text}` (alert).

### Legacy mode

Sketches written against older app versions can call
`setProtocol(BluetoothSerialConnectLegacy)` to use the original `B{n}` / `J{n}:…` / `@` / `#`
format. The current app only emits the modern protocol, so a legacy sketch will not
receive button/joystick messages from it until updated to
`setProtocol(BluetoothSerialConnectModern)` (the default).

## Diagnostics

`readSerial()` is non-blocking. With `verbose` enabled, received lines are echoed to the
Arduino `Serial` monitor as `BSC received: \`…\``. Parser/configuration problems are
available via `lastError()`, `lastErrorMessage()`, and `clearError()`.

## License

Released into the public domain under the Unlicense. See `LICENSE`.
