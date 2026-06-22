#include "MAX7219Driver.h"

#define REG_NOOP 0x00
#define REG_DIGIT0 0x01
#define REG_DECODEMODE 0x09
#define REG_INTENSITY 0x0A
#define REG_SCANLIMIT 0x0B
#define REG_SHUTDOWN 0x0C
#define REG_DISPLAYTEST 0x0F

MAX7219Driver::MAX7219Driver(uint8_t csPin, uint8_t digits) : csPin(csPin), digitCount(digits) {}

bool MAX7219Driver::init()
{
    pinMode(csPin, OUTPUT);
    digitalWrite(csPin, HIGH);
    SPI.begin();
    writeRegister(REG_DISPLAYTEST, 0);
    writeRegister(REG_DECODEMODE, 0xFF);
    writeRegister(REG_SCANLIMIT, digitCount - 1);
    setBrightness(brightness);
    writeRegister(REG_SHUTDOWN, 1);
    clear();
    initialized = true;
    return true;
}

void MAX7219Driver::shutdown()
{
    if (!initialized)
        return;

    // Clear displayed data first
    clear();

    // Put MAX7219 into shutdown mode
    writeRegister(
        REG_SHUTDOWN,
        0);

    // Reset CS pin to safe state
    digitalWrite(csPin, HIGH);
    pinMode(csPin, INPUT);

    initialized = false;
}

bool MAX7219Driver::available() const { return initialized; }

uint8_t MAX7219Driver::digits() const { return digitCount; }

void MAX7219Driver::writeRegister(uint8_t reg, uint8_t data)
{
    digitalWrite(csPin, LOW);
    SPI.transfer(reg);
    SPI.transfer(data);
    digitalWrite(csPin, HIGH);
}

void MAX7219Driver::clear()
{
    for (uint8_t i = 1; i <= digitCount; i++)
    {
        writeRegister(i, 0x0F);
    }
}

void MAX7219Driver::setDigit(uint8_t digit, char value, bool dot)
{
    if (digit >= digitCount)
        return;
    uint8_t code = 0x0F;
    if (value >= '0' && value <= '9')
        code = value - '0';
    else if (value == '-')
        code = 0x0A;
    else if (value == 'E')
        code = 0x0B;
    else if (value == ' ')
        code = 0x0F;
    if (dot)
        code |= 0x80;
    writeRegister(REG_DIGIT0 + digit, code);
}

void MAX7219Driver::setBrightness(uint8_t value)
{
    if (value > 15)
        value = 15;
    brightness = value;
    writeRegister(REG_INTENSITY, brightness);
}