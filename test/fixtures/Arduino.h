// Minimal Arduino surface for host-compiling and testing BluetoothSerialConnect.
// Mirrors the real Print -> Stream -> HardwareSerial hierarchy so the library can
// hold a Stream* and talk to either a HardwareSerial or a SoftwareSerial.
#ifndef TEST_FIXTURES_ARDUINO_H
#define TEST_FIXTURES_ARDUINO_H

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>

#define PI 3.14159265358979323846
#define F(value) value

class String {
private:
    std::string _value;

public:
    String() : _value() {}
    String(const char* value) : _value(value == nullptr ? "" : value) {}
    String(const std::string& value) : _value(value) {}

    const char* c_str() const {
        return _value.c_str();
    }
};

// Base output type: both HardwareSerial and SoftwareSerial inherit write/print from here.
class Print {
public:
    std::string outgoing;

    virtual ~Print() {}

    size_t write(uint8_t c) {
        outgoing += (char)c;
        return 1;
    }

    size_t write(const char* value) {
        if (value == nullptr) {
            return 0;
        }
        outgoing += value;
        return strlen(value);
    }

    size_t print(const char* value) {
        return write(value);
    }

    size_t print(int value) {
        std::string text = std::to_string(value);
        outgoing += text;
        return text.size();
    }

    size_t print(double value, int decimalPlaces = 2) {
        std::string text = std::to_string(value);
        size_t decimal = text.find('.');
        if (decimalPlaces == 0) {
            text.erase(decimal);
        } else if (decimal != std::string::npos) {
            text.resize(decimal + 1 + (size_t)decimalPlaces);
        }
        outgoing += text;
        return text.size();
    }

    size_t println(const char* value) {
        size_t count = write(value);
        outgoing += "\n";
        return count + 1;
    }
};

// Base input type: gives the library available()/read() without knowing the concrete port.
class Stream : public Print {
public:
    virtual int available() = 0;
    virtual int read() = 0;
    virtual int peek() { return -1; }
};

// A byte-queue stream used as both the test "module" and the global Serial.
class HardwareSerial : public Stream {
private:
    std::string _incoming;

public:
    void begin(int baudRate) {
        (void)baudRate;
    }

    int available() override {
        return (int)_incoming.size();
    }

    int read() override {
        if (_incoming.empty()) {
            return -1;
        }
        char c = _incoming[0];
        _incoming.erase(0, 1);
        return c;
    }

    void pushIncoming(const char* value) {
        if (value != nullptr) {
            _incoming += value;
        }
    }
};

// Stand-in for the Arduino SoftwareSerial library: a separate Stream subclass
// (a sibling of HardwareSerial) so we can exercise the generic Stream& constructor.
class SoftwareSerial : public Stream {
private:
    std::string _incoming;

public:
    SoftwareSerial() {}
    SoftwareSerial(int rxPin, int txPin) {
        (void)rxPin;
        (void)txPin;
    }

    void begin(int baudRate) {
        (void)baudRate;
    }

    int available() override {
        return (int)_incoming.size();
    }

    int read() override {
        if (_incoming.empty()) {
            return -1;
        }
        char c = _incoming[0];
        _incoming.erase(0, 1);
        return c;
    }

    void pushIncoming(const char* value) {
        if (value != nullptr) {
            _incoming += value;
        }
    }
};

extern HardwareSerial Serial;

#endif
