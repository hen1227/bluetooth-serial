/*
  BluetoothSerialConnect.h - Bluetooth Serial Connect for Arduino
  Created by Henry Abrahamsen, March 3, 2023
  Released into the public domain.
*/

#ifndef BLUETOOTH_SERIAL_CONNECT_H
#define BLUETOOTH_SERIAL_CONNECT_H

#include <Arduino.h>
#include "BluetoothSerialConnectJoystick.h"

enum BluetoothSerialConnectError {
    BluetoothSerialConnectNoError = 0,
    BluetoothSerialConnectInputOverflow,
    BluetoothSerialConnectMessageOverflow,
    BluetoothSerialConnectInvalidButtonId,
    BluetoothSerialConnectInvalidJoystickMessage,
    BluetoothSerialConnectInvalidJoystickId,
    BluetoothSerialConnectInvalidProtocolConfig,
    BluetoothSerialConnectInvalidModernMessage,
    BluetoothSerialConnectInvalidChannel
};

/// Which wire protocol the library speaks. The current app uses the modern typed protocol.
/// Legacy mode remains available for sketches paired with older app versions.
enum BluetoothSerialConnectProtocol {
    BluetoothSerialConnectLegacy = 0,
    BluetoothSerialConnectModern
};

/// Directions reported by a modern D-Pad component.
enum BluetoothSerialConnectDpadDirection {
    BluetoothSerialConnectDpadUp = 0,
    BluetoothSerialConnectDpadDown,
    BluetoothSerialConnectDpadLeft,
    BluetoothSerialConnectDpadRight,
    BluetoothSerialConnectDpadUpLeft,
    BluetoothSerialConnectDpadUpRight,
    BluetoothSerialConnectDpadDownLeft,
    BluetoothSerialConnectDpadDownRight,
    BluetoothSerialConnectDpadCenter
};

/// Highest component channel supported by the library. Channels are namespaced by type.
static const int BluetoothSerialConnectMaxChannel = 19;


class BluetoothSerialConnect {
private:
    static constexpr int kMaxButtons = 20;
    static constexpr int kMaxJoysticks = 20;
    static constexpr size_t kInputBufferSize = 96;
    static constexpr size_t kMessageBufferSize = 96;
    static constexpr size_t kMaxPrefixLength = 8;
    static constexpr size_t kMaxSuffixLength = 8;

    Stream* _stream;                 // used for all reads/writes (HardwareSerial and SoftwareSerial both derive from Stream)
    HardwareSerial* _hardwareSerial;  // non-null only when constructed from a HardwareSerial, so begin() can open the port
    bool _verbose;
    bool _buttonsList[kMaxButtons];
    bool _joysticksUpdatedList[kMaxJoysticks];
    BluetoothSerialConnectJoystick _joysticksList[kMaxJoysticks];

    char _inputBuffer[kInputBufferSize];
    size_t _inputBufferLength;

    char _messageBuffer[kMessageBufferSize];
    bool _messageAvailable;

    char _buttonPrefix[kMaxPrefixLength];
    char _joystickPrefix[kMaxPrefixLength];
    char _consolePrefix[kMaxPrefixLength];
    char _alertPrefix[kMaxPrefixLength];
    char _displayPrefix[kMaxPrefixLength];

    char _buttonSuffix[kMaxSuffixLength];
    char _joystickSuffix[kMaxSuffixLength];
    char _consoleSuffix[kMaxSuffixLength];
    char _outputSuffix[kMaxSuffixLength];

    BluetoothSerialConnectError _lastError;

    // Modern protocol -------------------------------------------------------------------
    static constexpr int kMaxChannels = BluetoothSerialConnectMaxChannel + 1;
    BluetoothSerialConnectProtocol _protocol;

    // Persistent modern state (survives across readSerial calls).
    bool _modernButtonHeld[kMaxChannels];
    bool _modernToggle[kMaxChannels];
    double _modernSlider[kMaxChannels];
    uint16_t _modernDpadHeld[kMaxChannels];     // bitmask of held directions

    // Transient modern state (cleared at the start of each readSerial, like legacy buttons).
    bool _modernButtonPressed[kMaxChannels];
    bool _modernButtonReleased[kMaxChannels];
    bool _modernButtonRepeated[kMaxChannels];
    bool _modernToggleUpdated[kMaxChannels];
    bool _modernSliderUpdated[kMaxChannels];
    uint16_t _modernDpadPressed[kMaxChannels];  // bitmask of directions pressed this cycle
    uint16_t _modernDpadReleased[kMaxChannels]; // bitmask of directions released this cycle
    uint16_t _modernDpadRepeated[kMaxChannels]; // bitmask of directions repeated this cycle

    void resetInputBuffer();
    void clearTransientInputState();
    void clearModernTransientState();
    void processIncomingByte(char c);
    bool hasCompleteMessage(size_t* suffixLength);
    void processMessage(char* message);
    bool parseButtonMessage(const char* body);
    bool parseJoystickMessage(const char* body);
    bool parseModernMessage(char* message);
    bool channelInRange(long channel);
    int dpadDirectionIndex(const char* text);
    void unescapeInPlace(char* text);
    void writeEscaped(const char* text);
    void setLastError(BluetoothSerialConnectError error);
    bool copyProtocolText(char* destination, size_t destinationSize, const char* value);
    bool startsWith(const char* message, const char* prefix);
    bool endsWith(const char* message, size_t messageLength, const char* suffix);

public:
    /**
     * Constructor for a hardware UART (e.g., Serial1). This is the common case;
     * begin() will open the port for you.
     *
     * @param serial Reference to a HardwareSerial object (e.g., Serial1).
     * @param verbose Indicates if debug messages should be printed to the Serial.
     */
    BluetoothSerialConnect(HardwareSerial& serial, bool verbose = true);

    /**
     * Constructor for any serial-like Stream — most commonly a SoftwareSerial, but
     * also any other class deriving from Arduino's Stream.
     *
     * The library does not include (and so does not require) the SoftwareSerial
     * library; include it in your sketch and pass your SoftwareSerial instance here.
     * Because the port type is not known to the library, you must call begin() on
     * that stream yourself (e.g., `mySerial.begin(9600);`) before reading — see the
     * Software_Serial example.
     *
     * @param stream Reference to a started serial-like Stream (e.g., a SoftwareSerial).
     * @param verbose Indicates if debug messages should be printed to the Serial.
     */
    BluetoothSerialConnect(Stream& stream, bool verbose = true);

    /**
     * Initialize the serial communication.
     *
     * Typically called within the Arduino setup() function.
     *
     * When the object was constructed from a HardwareSerial, this opens that port at
     * the given baud rate. When constructed from a generic Stream/SoftwareSerial, the
     * port is not opened here (call begin() on that stream yourself); this only emits
     * the verbose start message.
     *
     * @param baudRate The baud rate for serial communication. Defaults to 9600.
     */
    void begin(int baudRate = 9600);

    /**
     * Reads and processes input from the Bluetooth serial connection.
     *
     * Typically called at the beginning of the Arduino loop() function.
     */
    void readSerial();

    /**
     * Reads and processes input from the Bluetooth serial connection using a
     * single-character terminator. The timeout argument is kept for source
     * compatibility; parsing is non-blocking and incremental.
     *
     * @param terminator Character that indicates the end of a message. Default is '\n'.
     * @param terminatingTimeout Ignored. Kept for compatibility with v1 sketches.
     */
    void readSerial(char terminator, int terminatingTimeout = 0);

    /**
     * Sends a raw message to the connected iOS device.
     * Bluetooth Serial Connect does not process the message contents.
     *
     * @param message The content to be sent.
     */
    void writeSerial(const char* message);
    void writeSerial(const String& message);

    /**
     * Sends an alert message to the connected Bluetooth device. Longer messages may experience delays.
     *
     * @param message Alert content.
     */
    void sendAlert(const char* message);
    void sendAlert(const String& message);

    /**
     * Displays a message on the connected app.
     *
     * @param message The content to display on the app.
     * @param id Identifier for a specific display element in the app.
     */
    void setDisplay(const char* message, int id);
    void setDisplay(const String& message, int id);

    /**
     * Selects the wire protocol. Defaults to `BluetoothSerialConnectModern` to match the current
     * app. Select `BluetoothSerialConnectLegacy` only when pairing with an older app version that
     * sends the original B/J/@/# format.
     */
    void setProtocol(BluetoothSerialConnectProtocol mode);
    BluetoothSerialConnectProtocol protocol();

    /** Sets a plain text label (modern `L{id}:text`; falls back to a display in legacy). */
    void setLabel(const char* message, int id);
    void setLabel(const String& message, int id);

    /** Sets an indicator LED on/off (modern `I{id}:0|1`). */
    void setIndicator(bool on, int id);

    /** Sets a gauge's value (modern `G{id}:value`). */
    void setGauge(double value, int id, int decimalPlaces = 2);

    /**
     * Configure protocol text to match custom board settings in the app.
     *
     * Prefixes and suffixes are intentionally short because the wire protocol is
     * designed for small ASCII messages over BLE serial modules.
     */
    bool setButtonPrefix(const char* prefix);
    bool setJoystickPrefix(const char* prefix);
    bool setConsolePrefix(const char* prefix);
    bool setAlertPrefix(const char* prefix);
    bool setDisplayPrefix(const char* prefix);
    bool setButtonSuffix(const char* suffix);
    bool setJoystickSuffix(const char* suffix);
    bool setConsoleSuffix(const char* suffix);
    bool setInputSuffix(const char* suffix);
    bool setOutputSuffix(const char* suffix);

    /**
     * Clears the record of pressed buttons.
     *
     * After calling this method, all button states are set to "not pressed" until
     * they are read again by readSerial().
     * Typically called at the end of the Arduino loop() function.
     */
    void clearBuffer();

    /**
     * Checks if a specific button is currently pressed.
     *
     * @param id The identifier for the button as defined in the app.
     * @return True if the button is pressed, false otherwise.
     */
    bool isButtonPressed(int id);

    /**
     * Checks if a specific joystick has been updated since last call.
     *
     * @param id The identifier for the joystick as defined in the app.
     * @return True if the joystick is updated, false otherwise.
     */
    bool isJoystickUpdated(int id);


    /**
     * Gets string messages from app. Sent through the console section of the board.
     *
     * Reading the message clears the unread-message flag.
     *
     * @return Latest message from app's console.
     */
    String getMessage();

    /**
     * Checks whether an unread console message is available.
     *
     * @return True if getMessage() can return a new console message.
     */
    bool hasMessage();

    /**
     * Retrieves the current state of a specific joystick.
     *
     * @param id The identifier for the joystick as defined in the app.
     * @return A BluetoothSerialConnectJoystick object representing the joystick's current state.
     */
    BluetoothSerialConnectJoystick getJoystick(int id);

    // Modern-protocol inputs -------------------------------------------------------------
    // (Buttons/joysticks also work via isButtonPressed/getJoystick above; in modern mode a
    //  button stays "pressed" until its release event, unlike the per-read legacy flag.)

    /** True the cycle a modern button press (`B{ch}:D`) was received. */
    bool wasButtonPressed(int channel);

    /** True the cycle a modern button release (`B{ch}:U`) was received. */
    bool wasButtonReleased(int channel);

    /** True the cycle a modern button repeat (`B{ch}:R`) was received. */
    bool wasButtonRepeated(int channel);

    /** Current state of a modern toggle/switch component. */
    bool isToggleOn(int channel);

    /** True the cycle a modern toggle value (`T{ch}:0|1`) was received. */
    bool wasToggleUpdated(int channel);

    /** True the cycle a modern slider value (`S{ch}:value`) was received. */
    bool isSliderUpdated(int channel);

    /** Latest value of a modern slider component. */
    double getSlider(int channel);

    /** Whether a D-Pad direction is currently held, or was pressed this read cycle. */
    bool isDpadPressed(int channel, BluetoothSerialConnectDpadDirection direction);

    /** True the cycle a D-Pad direction down event was received. */
    bool wasDpadPressed(int channel, BluetoothSerialConnectDpadDirection direction);

    /** True the cycle a D-Pad direction release event was received. */
    bool wasDpadReleased(int channel, BluetoothSerialConnectDpadDirection direction);

    /** True the cycle a D-Pad direction repeat event was received. */
    bool wasDpadRepeated(int channel, BluetoothSerialConnectDpadDirection direction);

    /**
     * Returns the most recent parser/configuration error.
     */
    BluetoothSerialConnectError lastError();

    /**
     * Returns a short human-readable description of lastError().
     */
    const char* lastErrorMessage();

    /**
     * Clears the most recent parser/configuration error.
     */
    void clearError();
};

#endif
