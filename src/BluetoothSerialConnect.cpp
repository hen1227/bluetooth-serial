/*
  BluetoothSerialConnect.cpp - Bluetooth Serial Connect for Arduino
  Created by Henry Abrahamsen, March 3, 2023
  Released into the public domain.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "BluetoothSerialConnectJoystick.h"
#include "BluetoothSerialConnect.h"

BluetoothSerialConnect::BluetoothSerialConnect(Stream& stream, bool verbose /*= true*/) :
    _stream(&stream),
    _hardwareSerial(nullptr),
    _verbose(verbose),
    _inputBufferLength(0),
    _messageAvailable(false),
    _lastError(BluetoothSerialConnectNoError) {
    _inputBuffer[0] = '\0';
    _messageBuffer[0] = '\0';

    _protocol = BluetoothSerialConnectModern;
    for (int i = 0; i < kMaxChannels; i++) {
        _modernButtonHeld[i] = false;
        _modernToggle[i] = false;
        _modernSlider[i] = 0;
        _modernDpadHeld[i] = 0;
        _modernButtonPressed[i] = false;
        _modernButtonReleased[i] = false;
        _modernButtonRepeated[i] = false;
        _modernToggleUpdated[i] = false;
        _modernSliderUpdated[i] = false;
        _modernDpadPressed[i] = 0;
        _modernDpadReleased[i] = 0;
        _modernDpadRepeated[i] = 0;
    }

    clearTransientInputState();

    copyProtocolText(_buttonPrefix, sizeof(_buttonPrefix), "B");
    copyProtocolText(_joystickPrefix, sizeof(_joystickPrefix), "J");
    copyProtocolText(_consolePrefix, sizeof(_consolePrefix), "@");
    copyProtocolText(_alertPrefix, sizeof(_alertPrefix), "@");
    copyProtocolText(_displayPrefix, sizeof(_displayPrefix), "#");

    copyProtocolText(_buttonSuffix, sizeof(_buttonSuffix), "\n");
    copyProtocolText(_joystickSuffix, sizeof(_joystickSuffix), "\n");
    copyProtocolText(_consoleSuffix, sizeof(_consoleSuffix), "\n");
    copyProtocolText(_outputSuffix, sizeof(_outputSuffix), "\n");
}

BluetoothSerialConnect::BluetoothSerialConnect(HardwareSerial& serial, bool verbose /*= true*/) :
    BluetoothSerialConnect(static_cast<Stream&>(serial), verbose) {
    // Remember the concrete hardware port so begin() can open it.
    _hardwareSerial = &serial;
}

void BluetoothSerialConnect::begin(int baudRate /*= 9600*/) {
    // Only hardware UARTs are opened here. A SoftwareSerial (or any other Stream)
    // must be started by the sketch before reading; see the Software_Serial example.
    if (_hardwareSerial != nullptr) {
        _hardwareSerial->begin(baudRate);
    }
    if (_verbose) {
        Serial.println(F("Bluetooth Serial Connect begun"));
    }
}

void BluetoothSerialConnect::readSerial() {
    clearTransientInputState();
    while (_stream->available() > 0) {
        processIncomingByte((char)_stream->read());
    }
}

void BluetoothSerialConnect::readSerial(char terminator, int terminatingTimeout /*= 0*/) {
    (void)terminatingTimeout;

    char suffix[2];
    suffix[0] = terminator;
    suffix[1] = '\0';
    setInputSuffix(suffix);

    readSerial();
}

void BluetoothSerialConnect::processIncomingByte(char c) {
    if (_inputBufferLength >= kInputBufferSize - 1) {
        setLastError(BluetoothSerialConnectInputOverflow);
        resetInputBuffer();
        return;
    }

    _inputBuffer[_inputBufferLength++] = c;
    _inputBuffer[_inputBufferLength] = '\0';

    size_t suffixLength = 0;
    if (!hasCompleteMessage(&suffixLength)) {
        return;
    }

    _inputBuffer[_inputBufferLength - suffixLength] = '\0';
    while (_inputBufferLength > suffixLength && _inputBuffer[_inputBufferLength - suffixLength - 1] == '\r') {
        _inputBuffer[--_inputBufferLength - suffixLength] = '\0';
    }

    if (_verbose) {
        Serial.print(F("BSC received: `"));
        Serial.print(_inputBuffer);
        Serial.println(F("`"));
    }

    processMessage(_inputBuffer);
    resetInputBuffer();
}

bool BluetoothSerialConnect::hasCompleteMessage(size_t* suffixLength) {
    // Modern frames are always newline-terminated; flush as soon as a '\n' arrives.
    if (_protocol == BluetoothSerialConnectModern) {
        if (endsWith(_inputBuffer, _inputBufferLength, "\n")) {
            *suffixLength = 1;
            return true;
        }
        return false;
    }

    bool isButtonMessage = startsWith(_inputBuffer, _buttonPrefix);
    bool isJoystickMessage = startsWith(_inputBuffer, _joystickPrefix);
    bool isConsoleMessage = startsWith(_inputBuffer, _consolePrefix);

    if (isButtonMessage && endsWith(_inputBuffer, _inputBufferLength, _buttonSuffix)) {
        *suffixLength = strlen(_buttonSuffix);
        return true;
    }

    if (isJoystickMessage && endsWith(_inputBuffer, _inputBufferLength, _joystickSuffix)) {
        *suffixLength = strlen(_joystickSuffix);
        return true;
    }

    if (isConsoleMessage && endsWith(_inputBuffer, _inputBufferLength, _consoleSuffix)) {
        *suffixLength = strlen(_consoleSuffix);
        return true;
    }

    if (isButtonMessage || isJoystickMessage || isConsoleMessage) {
        return false;
    }

    // Unknown messages are flushed at any known suffix so the buffer cannot
    // remain stuck behind data the library intentionally ignores.
    const char* suffixes[] = { _buttonSuffix, _joystickSuffix, _consoleSuffix };
    size_t bestLength = 0;

    for (size_t i = 0; i < sizeof(suffixes) / sizeof(suffixes[0]); i++) {
        const char* suffix = suffixes[i];
        size_t currentLength = strlen(suffix);
        if (currentLength == 0 || currentLength > _inputBufferLength || currentLength <= bestLength) {
            continue;
        }

        if (endsWith(_inputBuffer, _inputBufferLength, suffix)) {
            bestLength = currentLength;
        }
    }

    if (bestLength == 0) {
        return false;
    }

    *suffixLength = bestLength;
    return true;
}

void BluetoothSerialConnect::processMessage(char* message) {
    if (message == nullptr || message[0] == '\0') {
        return;
    }

    if (_protocol == BluetoothSerialConnectModern) {
        parseModernMessage(message);
        return;
    }

    if (startsWith(message, _buttonPrefix)) {
        parseButtonMessage(message + strlen(_buttonPrefix));
        return;
    }

    if (startsWith(message, _joystickPrefix)) {
        parseJoystickMessage(message + strlen(_joystickPrefix));
        return;
    }

    if (startsWith(message, _consolePrefix)) {
        const char* body = message + strlen(_consolePrefix);
        size_t bodyLength = strlen(body);
        if (bodyLength >= kMessageBufferSize) {
            strncpy(_messageBuffer, body, kMessageBufferSize - 1);
            _messageBuffer[kMessageBufferSize - 1] = '\0';
            setLastError(BluetoothSerialConnectMessageOverflow);
        } else {
            strcpy(_messageBuffer, body);
        }
        _messageAvailable = true;
    }
}

bool BluetoothSerialConnect::parseButtonMessage(const char* body) {
    char* end = nullptr;
    long id = strtol(body, &end, 10);

    if (body == end || *end != '\0' || id < 0 || id >= kMaxButtons) {
        setLastError(BluetoothSerialConnectInvalidButtonId);
        return false;
    }

    _buttonsList[id] = true;
    return true;
}

bool BluetoothSerialConnect::parseJoystickMessage(const char* body) {
    char* end = nullptr;
    long id = strtol(body, &end, 10);

    if (body == end || *end != ':') {
        setLastError(BluetoothSerialConnectInvalidJoystickMessage);
        return false;
    }

    if (id < 0 || id >= kMaxJoysticks) {
        setLastError(BluetoothSerialConnectInvalidJoystickId);
        return false;
    }

    const char* rotationStart = end + 1;
    double rotation = strtod(rotationStart, &end);
    if (rotationStart == end || *end != ',') {
        setLastError(BluetoothSerialConnectInvalidJoystickMessage);
        return false;
    }

    const char* magnitudeStart = end + 1;
    double magnitude = strtod(magnitudeStart, &end);
    if (magnitudeStart == end || *end != '\0') {
        setLastError(BluetoothSerialConnectInvalidJoystickMessage);
        return false;
    }

    _joysticksList[id].updateValues(rotation, magnitude);
    _joysticksUpdatedList[id] = true;
    return true;
}

void BluetoothSerialConnect::writeSerial(const char* message) {
    if (message == nullptr) {
        return;
    }
    _stream->write(message);
}

void BluetoothSerialConnect::writeSerial(const String& message) {
    writeSerial(message.c_str());
}

void BluetoothSerialConnect::sendAlert(const char* message) {
    if (_protocol == BluetoothSerialConnectModern) {
        writeSerial("A:");
        writeEscaped(message);
        writeSerial("\n");
        return;
    }
    writeSerial(_alertPrefix);
    writeSerial(message == nullptr ? "" : message);
    writeSerial(_outputSuffix);
}

void BluetoothSerialConnect::sendAlert(const String& message) {
    sendAlert(message.c_str());
}

void BluetoothSerialConnect::setDisplay(const char* message, int id) {
    if (_protocol == BluetoothSerialConnectModern) {
        writeSerial("D");
        _stream->print(id);
        writeSerial(":");
        writeEscaped(message);
        writeSerial("\n");
        return;
    }
    writeSerial(_displayPrefix);
    _stream->print(id);
    writeSerial(":");
    writeSerial(message == nullptr ? "" : message);
    writeSerial(_outputSuffix);
}

void BluetoothSerialConnect::setDisplay(const String& message, int id) {
    setDisplay(message.c_str(), id);
}

bool BluetoothSerialConnect::setButtonPrefix(const char* prefix) {
    return copyProtocolText(_buttonPrefix, sizeof(_buttonPrefix), prefix);
}

bool BluetoothSerialConnect::setJoystickPrefix(const char* prefix) {
    return copyProtocolText(_joystickPrefix, sizeof(_joystickPrefix), prefix);
}

bool BluetoothSerialConnect::setConsolePrefix(const char* prefix) {
    return copyProtocolText(_consolePrefix, sizeof(_consolePrefix), prefix);
}

bool BluetoothSerialConnect::setAlertPrefix(const char* prefix) {
    return copyProtocolText(_alertPrefix, sizeof(_alertPrefix), prefix);
}

bool BluetoothSerialConnect::setDisplayPrefix(const char* prefix) {
    return copyProtocolText(_displayPrefix, sizeof(_displayPrefix), prefix);
}

bool BluetoothSerialConnect::setButtonSuffix(const char* suffix) {
    return copyProtocolText(_buttonSuffix, sizeof(_buttonSuffix), suffix);
}

bool BluetoothSerialConnect::setJoystickSuffix(const char* suffix) {
    return copyProtocolText(_joystickSuffix, sizeof(_joystickSuffix), suffix);
}

bool BluetoothSerialConnect::setConsoleSuffix(const char* suffix) {
    return copyProtocolText(_consoleSuffix, sizeof(_consoleSuffix), suffix);
}

bool BluetoothSerialConnect::setInputSuffix(const char* suffix) {
    bool ok = true;
    ok = setButtonSuffix(suffix) && ok;
    ok = setJoystickSuffix(suffix) && ok;
    ok = setConsoleSuffix(suffix) && ok;
    return ok;
}

bool BluetoothSerialConnect::setOutputSuffix(const char* suffix) {
    return copyProtocolText(_outputSuffix, sizeof(_outputSuffix), suffix);
}

void BluetoothSerialConnect::clearBuffer() {
    clearTransientInputState();
}

void BluetoothSerialConnect::clearTransientInputState() {
    for (int i = 0; i < kMaxButtons; i++) {
        _buttonsList[i] = false;
    }
    for (int i = 0; i < kMaxJoysticks; i++) {
        _joysticksUpdatedList[i] = false;
    }
    clearModernTransientState();
}

void BluetoothSerialConnect::clearModernTransientState() {
    for (int i = 0; i < kMaxChannels; i++) {
        _modernButtonPressed[i] = false;
        _modernButtonReleased[i] = false;
        _modernButtonRepeated[i] = false;
        _modernToggleUpdated[i] = false;
        _modernSliderUpdated[i] = false;
        _modernDpadPressed[i] = 0;
        _modernDpadReleased[i] = 0;
        _modernDpadRepeated[i] = 0;
    }
}

bool BluetoothSerialConnect::isButtonPressed(int id) {
    if (_protocol == BluetoothSerialConnectModern) {
        return (id >= 0 && id < kMaxChannels) ? _modernButtonHeld[id] : false;
    }
    if (id >= 0 && id < kMaxButtons) {
        return _buttonsList[id];
    }
    return false;
}

bool BluetoothSerialConnect::isJoystickUpdated(int id) {
    if (id >= 0 && id < kMaxJoysticks) {
        return _joysticksUpdatedList[id];
    }
    return false;
}

String BluetoothSerialConnect::getMessage() {
    _messageAvailable = false;
    return String(_messageBuffer);
}

bool BluetoothSerialConnect::hasMessage() {
    return _messageAvailable;
}

BluetoothSerialConnectJoystick BluetoothSerialConnect::getJoystick(int id) {
    if (id >= 0 && id < kMaxJoysticks) {
        return _joysticksList[id];
    }
    return BluetoothSerialConnectJoystick(0, 0);
}

// MARK: - Modern protocol

void BluetoothSerialConnect::setProtocol(BluetoothSerialConnectProtocol mode) {
    _protocol = mode;
}

BluetoothSerialConnectProtocol BluetoothSerialConnect::protocol() {
    return _protocol;
}

bool BluetoothSerialConnect::channelInRange(long channel) {
    return channel >= 0 && channel < kMaxChannels;
}

int BluetoothSerialConnect::dpadDirectionIndex(const char* text) {
    if (text == nullptr) return -1;
    if (strcmp(text, "U") == 0) return BluetoothSerialConnectDpadUp;
    if (strcmp(text, "D") == 0) return BluetoothSerialConnectDpadDown;
    if (strcmp(text, "L") == 0) return BluetoothSerialConnectDpadLeft;
    if (strcmp(text, "R") == 0) return BluetoothSerialConnectDpadRight;
    if (strcmp(text, "UL") == 0) return BluetoothSerialConnectDpadUpLeft;
    if (strcmp(text, "UR") == 0) return BluetoothSerialConnectDpadUpRight;
    if (strcmp(text, "DL") == 0) return BluetoothSerialConnectDpadDownLeft;
    if (strcmp(text, "DR") == 0) return BluetoothSerialConnectDpadDownRight;
    if (strcmp(text, "C") == 0) return BluetoothSerialConnectDpadCenter;
    return -1;
}

void BluetoothSerialConnect::unescapeInPlace(char* text) {
    if (text == nullptr) return;
    char* read = text;
    char* write = text;
    while (*read != '\0') {
        if (*read == '\\' && *(read + 1) != '\0') {
            char next = *(read + 1);
            if (next == 'n') { *write++ = '\n'; read += 2; continue; }
            if (next == 'r') { *write++ = '\r'; read += 2; continue; }
            if (next == '\\') { *write++ = '\\'; read += 2; continue; }
        }
        *write++ = *read++;
    }
    *write = '\0';
}

void BluetoothSerialConnect::writeEscaped(const char* text) {
    if (text == nullptr) return;
    for (const char* p = text; *p != '\0'; ++p) {
        char c = *p;
        if (c == '\\') {
            writeSerial("\\\\");
        } else if (c == '\n') {
            writeSerial("\\n");
        } else if (c == '\r') {
            writeSerial("\\r");
        } else {
            char single[2] = { c, '\0' };
            writeSerial(single);
        }
    }
}

bool BluetoothSerialConnect::parseModernMessage(char* message) {
    if (message == nullptr || message[0] == '\0') {
        return false;
    }

    char type = message[0];

    // Console has no channel: `C:text`.
    if (type == 'C') {
        if (message[1] != ':') {
            setLastError(BluetoothSerialConnectInvalidModernMessage);
            return false;
        }
        char* body = message + 2;
        unescapeInPlace(body);
        if (strlen(body) >= kMessageBufferSize) {
            strncpy(_messageBuffer, body, kMessageBufferSize - 1);
            _messageBuffer[kMessageBufferSize - 1] = '\0';
            setLastError(BluetoothSerialConnectMessageOverflow);
        } else {
            strcpy(_messageBuffer, body);
        }
        _messageAvailable = true;
        return true;
    }

    char* cursor = message + 1;
    char* end = nullptr;
    long channel = strtol(cursor, &end, 10);
    if (cursor == end || *end != ':') {
        setLastError(BluetoothSerialConnectInvalidModernMessage);
        return false;
    }
    char* payload = end + 1;

    if (!channelInRange(channel)) {
        setLastError(BluetoothSerialConnectInvalidChannel);
        return false;
    }
    int ch = (int)channel;

    switch (type) {
        case 'B': {
            char event = payload[0];
            if ((event == 'D' || event == 'R') && payload[1] == '\0') {
                _modernButtonHeld[ch] = true;
                if (event == 'D') {
                    _modernButtonPressed[ch] = true;
                } else {
                    _modernButtonRepeated[ch] = true;
                }
                return true;
            }
            if (event == 'U' && payload[1] == '\0') {
                _modernButtonHeld[ch] = false;
                _modernButtonReleased[ch] = true;
                return true;
            }
            setLastError(BluetoothSerialConnectInvalidModernMessage);
            return false;
        }
        case 'J': {
            char* e = nullptr;
            double rotation = strtod(payload, &e);
            if (payload == e || *e != ',') {
                setLastError(BluetoothSerialConnectInvalidJoystickMessage);
                return false;
            }
            const char* magnitudeStart = e + 1;
            double magnitude = strtod(magnitudeStart, &e);
            if (magnitudeStart == e || *e != '\0') {
                setLastError(BluetoothSerialConnectInvalidJoystickMessage);
                return false;
            }
            _joysticksList[ch].updateValues(rotation, magnitude);
            _joysticksUpdatedList[ch] = true;
            return true;
        }
        case 'S': {
            char* e = nullptr;
            double value = strtod(payload, &e);
            if (payload == e || *e != '\0') {
                setLastError(BluetoothSerialConnectInvalidModernMessage);
                return false;
            }
            _modernSlider[ch] = value;
            _modernSliderUpdated[ch] = true;
            return true;
        }
        case 'T': {
            if (strcmp(payload, "0") != 0 && strcmp(payload, "1") != 0) {
                setLastError(BluetoothSerialConnectInvalidModernMessage);
                return false;
            }
            _modernToggle[ch] = payload[0] == '1';
            _modernToggleUpdated[ch] = true;
            return true;
        }
        case 'P': {
            char* comma = strchr(payload, ',');
            if (comma != nullptr) {
                *comma = '\0';
            }
            int direction = dpadDirectionIndex(payload);
            if (direction < 0) {
                setLastError(BluetoothSerialConnectInvalidModernMessage);
                return false;
            }
            uint16_t bit = (uint16_t)(1u << direction);
            if (comma == nullptr) {
                _modernDpadPressed[ch] |= bit;
                return true;
            }

            char event = comma[1];
            if (event == '\0' || comma[2] != '\0') {
                setLastError(BluetoothSerialConnectInvalidModernMessage);
                return false;
            }
            if (event == 'D') {
                _modernDpadHeld[ch] |= bit;
                _modernDpadPressed[ch] |= bit;
            } else if (event == 'R') {
                _modernDpadHeld[ch] |= bit;
                _modernDpadRepeated[ch] |= bit;
            } else if (event == 'U') {
                _modernDpadHeld[ch] = (uint16_t)(_modernDpadHeld[ch] & ~bit);
                _modernDpadReleased[ch] |= bit;
            } else {
                setLastError(BluetoothSerialConnectInvalidModernMessage);
                return false;
            }
            return true;
        }
        default:
            setLastError(BluetoothSerialConnectInvalidModernMessage);
            return false;
    }
}

bool BluetoothSerialConnect::wasButtonPressed(int channel) {
    return (channel >= 0 && channel < kMaxChannels) ? _modernButtonPressed[channel] : false;
}

bool BluetoothSerialConnect::wasButtonReleased(int channel) {
    return (channel >= 0 && channel < kMaxChannels) ? _modernButtonReleased[channel] : false;
}

bool BluetoothSerialConnect::wasButtonRepeated(int channel) {
    return (channel >= 0 && channel < kMaxChannels) ? _modernButtonRepeated[channel] : false;
}

bool BluetoothSerialConnect::isToggleOn(int channel) {
    return (channel >= 0 && channel < kMaxChannels) ? _modernToggle[channel] : false;
}

bool BluetoothSerialConnect::wasToggleUpdated(int channel) {
    return (channel >= 0 && channel < kMaxChannels) ? _modernToggleUpdated[channel] : false;
}

bool BluetoothSerialConnect::isSliderUpdated(int channel) {
    return (channel >= 0 && channel < kMaxChannels) ? _modernSliderUpdated[channel] : false;
}

double BluetoothSerialConnect::getSlider(int channel) {
    return (channel >= 0 && channel < kMaxChannels) ? _modernSlider[channel] : 0.0;
}

bool BluetoothSerialConnect::isDpadPressed(int channel, BluetoothSerialConnectDpadDirection direction) {
    if (channel < 0 || channel >= kMaxChannels ||
        direction < BluetoothSerialConnectDpadUp || direction > BluetoothSerialConnectDpadCenter) return false;
    uint16_t pressed = (uint16_t)(_modernDpadHeld[channel] | _modernDpadPressed[channel]);
    return (pressed >> (int)direction) & 1u;
}

bool BluetoothSerialConnect::wasDpadPressed(int channel, BluetoothSerialConnectDpadDirection direction) {
    if (channel < 0 || channel >= kMaxChannels ||
        direction < BluetoothSerialConnectDpadUp || direction > BluetoothSerialConnectDpadCenter) return false;
    return (_modernDpadPressed[channel] >> (int)direction) & 1u;
}

bool BluetoothSerialConnect::wasDpadReleased(int channel, BluetoothSerialConnectDpadDirection direction) {
    if (channel < 0 || channel >= kMaxChannels ||
        direction < BluetoothSerialConnectDpadUp || direction > BluetoothSerialConnectDpadCenter) return false;
    return (_modernDpadReleased[channel] >> (int)direction) & 1u;
}

bool BluetoothSerialConnect::wasDpadRepeated(int channel, BluetoothSerialConnectDpadDirection direction) {
    if (channel < 0 || channel >= kMaxChannels ||
        direction < BluetoothSerialConnectDpadUp || direction > BluetoothSerialConnectDpadCenter) return false;
    return (_modernDpadRepeated[channel] >> (int)direction) & 1u;
}

void BluetoothSerialConnect::setLabel(const char* message, int id) {
    if (_protocol == BluetoothSerialConnectModern) {
        writeSerial("L");
        _stream->print(id);
        writeSerial(":");
        writeEscaped(message);
        writeSerial("\n");
        return;
    }
    setDisplay(message, id);
}

void BluetoothSerialConnect::setLabel(const String& message, int id) {
    setLabel(message.c_str(), id);
}

void BluetoothSerialConnect::setIndicator(bool on, int id) {
    if (_protocol == BluetoothSerialConnectModern) {
        writeSerial("I");
        _stream->print(id);
        writeSerial(on ? ":1\n" : ":0\n");
        return;
    }
    setDisplay(on ? "1" : "0", id);
}

void BluetoothSerialConnect::setGauge(double value, int id, int decimalPlaces) {
    int digits = decimalPlaces < 0 ? 0 : decimalPlaces;
    if (_protocol == BluetoothSerialConnectModern) {
        writeSerial("G");
        _stream->print(id);
        writeSerial(":");
        _stream->print(value, digits);
        writeSerial("\n");
        return;
    }
    writeSerial(_displayPrefix);
    _stream->print(id);
    writeSerial(":");
    _stream->print(value, digits);
    writeSerial(_outputSuffix);
}

BluetoothSerialConnectError BluetoothSerialConnect::lastError() {
    return _lastError;
}

const char* BluetoothSerialConnect::lastErrorMessage() {
    switch (_lastError) {
        case BluetoothSerialConnectNoError:
            return "No error";
        case BluetoothSerialConnectInputOverflow:
            return "Input message exceeded Bluetooth Serial Connect buffer";
        case BluetoothSerialConnectMessageOverflow:
            return "Console message exceeded Bluetooth Serial Connect buffer";
        case BluetoothSerialConnectInvalidButtonId:
            return "Invalid button id";
        case BluetoothSerialConnectInvalidJoystickMessage:
            return "Invalid joystick message";
        case BluetoothSerialConnectInvalidJoystickId:
            return "Invalid joystick id";
        case BluetoothSerialConnectInvalidProtocolConfig:
            return "Invalid protocol configuration";
        case BluetoothSerialConnectInvalidModernMessage:
            return "Invalid modern protocol message";
        case BluetoothSerialConnectInvalidChannel:
            return "Channel out of range";
        default:
            return "Unknown Bluetooth Serial Connect error";
    }
}

void BluetoothSerialConnect::clearError() {
    _lastError = BluetoothSerialConnectNoError;
}

void BluetoothSerialConnect::resetInputBuffer() {
    _inputBufferLength = 0;
    _inputBuffer[0] = '\0';
}

void BluetoothSerialConnect::setLastError(BluetoothSerialConnectError error) {
    _lastError = error;
    if (_verbose && error != BluetoothSerialConnectNoError) {
        Serial.print(F("Bluetooth Serial Connect error: "));
        Serial.println(lastErrorMessage());
    }
}

bool BluetoothSerialConnect::copyProtocolText(char* destination, size_t destinationSize, const char* value) {
    if (destination == nullptr || destinationSize == 0 || value == nullptr || value[0] == '\0') {
        setLastError(BluetoothSerialConnectInvalidProtocolConfig);
        return false;
    }

    if (strlen(value) >= destinationSize) {
        setLastError(BluetoothSerialConnectInvalidProtocolConfig);
        return false;
    }

    strcpy(destination, value);
    return true;
}

bool BluetoothSerialConnect::startsWith(const char* message, const char* prefix) {
    if (message == nullptr || prefix == nullptr || prefix[0] == '\0') {
        return false;
    }

    return strncmp(message, prefix, strlen(prefix)) == 0;
}

bool BluetoothSerialConnect::endsWith(const char* message, size_t messageLength, const char* suffix) {
    if (message == nullptr || suffix == nullptr || suffix[0] == '\0') {
        return false;
    }

    size_t suffixLength = strlen(suffix);
    if (suffixLength > messageLength) {
        return false;
    }

    return strcmp(message + messageLength - suffixLength, suffix) == 0;
}
