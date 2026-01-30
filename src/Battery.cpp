#include "Battery.h"

// MAX17049 Register addresses
#define REG_VCELL    0x02  // Battery voltage (12-bit, upper)
#define REG_SOC      0x04  // State of charge
#define REG_MODE     0x06  // Mode register
#define REG_VERSION  0x08  // IC version
#define REG_HIBRT    0x0A  // Hibernate threshold
#define REG_CONFIG   0x0C  // Config (RCOMP, Sleep, Alert)
#define REG_VALRT    0x14  // Voltage alert thresholds
#define REG_CRATE    0x16  // Charge/discharge rate
#define REG_VRESET   0x18  // VRESET/ID
#define REG_STATUS   0x1A  // Status register
#define REG_CMD      0xFE  // Command register

// CONFIG register bits
#define CONFIG_SLEEP_BIT  0x0080  // Sleep mode enable

// Default RCOMP value (0x97 from datasheet)
#define DEFAULT_RCOMP  0x97

Battery& Battery::instance() {
    static Battery _instance;
    return _instance;
}

bool Battery::begin() {
    pinMode(BATT_ALERT_PIN, INPUT);

    // Check if device responds
    Wire.beginTransmission(MAX17049_ADDR);
    if (Wire.endTransmission() != 0) {
        Serial.println("MAX17049 not found!");
        _initialized = false;
        return false;
    }

    _initialized = true;

    // Read version to confirm communication
    uint16_t version = readReg(REG_VERSION);
    Serial.printlnf("MAX17049 detected, version: 0x%04X", version);

    // Set default RCOMP value
    // The upper byte of CONFIG is RCOMP, lower byte has other settings
    uint16_t config = readReg(REG_CONFIG);
    uint8_t currentRcomp = config >> 8;
    Serial.printlnf("Current RCOMP: 0x%02X", currentRcomp);

    // Only set RCOMP if it's at default, don't disturb learned values
    if (currentRcomp == 0x97) {
        Serial.println("RCOMP at default value");
    }

    // Print initial readings
    printRegisters();

    return true;
}

float Battery::getSoC() {
    if (!_initialized) return -1;

    // SOC register: upper byte = integer %, lower byte = 1/256th %
    uint16_t soc = readReg(REG_SOC);
    float percent = (float)(soc >> 8) + ((float)(soc & 0xFF) / 256.0f);

    // Clamp to valid range
    if (percent < 0.0f) percent = 0.0f;
    if (percent > 100.0f) percent = 100.0f;

    return percent;
}

float Battery::getVoltage() {
    if (!_initialized) return -1;

    uint16_t raw = readReg(REG_VCELL);

    // MAX17049 (2-cell): Full scale is 10.24V
    // Formula: voltage = raw * 10.24 / 65536
    // This equals: raw * 0.00015625 (which is 156.25uV per LSB)
    float voltage = (float)raw * 10.24f / 65536.0f;

    return voltage;
}

uint16_t Battery::getRawVoltage() {
    if (!_initialized) return 0;
    return readReg(REG_VCELL);
}

float Battery::getChangeRate() {
    if (!_initialized) return 0;

    // CRATE register: charge/discharge rate in %/hr
    // Signed 16-bit, 0.208%/hr per LSB
    int16_t crate = (int16_t)readReg(REG_CRATE);
    return (float)crate * 0.208f;
}

void Battery::quickStart() {
    if (!_initialized) return;

    // Quick Start: write 0x4000 to MODE register
    // Forces recalculation of SOC from current voltage
    // WARNING: This resets the learned model, use sparingly!
    writeReg(REG_MODE, 0x4000);
    Serial.println("MAX17049 Quick Start issued");
    delay(200);  // Wait for recalculation
}

void Battery::setRCOMP(uint8_t rcomp) {
    if (!_initialized) return;

    // RCOMP is the upper byte of CONFIG register
    uint16_t config = readReg(REG_CONFIG);
    config = (config & 0x00FF) | ((uint16_t)rcomp << 8);
    writeReg(REG_CONFIG, config);
}

uint8_t Battery::getRCOMP() {
    if (!_initialized) return 0;

    uint16_t config = readReg(REG_CONFIG);
    return (uint8_t)(config >> 8);
}

void Battery::sleep() {
    if (!_initialized) return;

    uint16_t config = readReg(REG_CONFIG);
    config |= CONFIG_SLEEP_BIT;
    writeReg(REG_CONFIG, config);
}

void Battery::wake() {
    if (!_initialized) return;

    uint16_t config = readReg(REG_CONFIG);
    config &= ~CONFIG_SLEEP_BIT;
    writeReg(REG_CONFIG, config);
}

void Battery::printRegisters() {
    if (!_initialized) {
        Serial.println("MAX17049 not initialized");
        return;
    }

    Serial.println("--- MAX17049 Registers ---");
    Serial.printlnf("VCELL:   0x%04X (%.3fV)", readReg(REG_VCELL), getVoltage());
    Serial.printlnf("SOC:     0x%04X (%.2f%%)", readReg(REG_SOC), getSoC());
    Serial.printlnf("MODE:    0x%04X", readReg(REG_MODE));
    Serial.printlnf("VERSION: 0x%04X", readReg(REG_VERSION));
    Serial.printlnf("HIBRT:   0x%04X", readReg(REG_HIBRT));
    Serial.printlnf("CONFIG:  0x%04X (RCOMP=0x%02X)", readReg(REG_CONFIG), getRCOMP());
    Serial.printlnf("CRATE:   0x%04X (%.1f%%/hr)", readReg(REG_CRATE), getChangeRate());
    Serial.printlnf("STATUS:  0x%04X", readReg(REG_STATUS));
    Serial.println("--------------------------");
}

uint16_t Battery::readReg(uint8_t reg) {
    Wire.beginTransmission(MAX17049_ADDR);
    Wire.write(reg);
    Wire.endTransmission(false);

    Wire.requestFrom(MAX17049_ADDR, 2);
    if (Wire.available() >= 2) {
        uint8_t msb = Wire.read();
        uint8_t lsb = Wire.read();
        return (msb << 8) | lsb;
    }
    return 0;
}

void Battery::writeReg(uint8_t reg, uint16_t value) {
    Wire.beginTransmission(MAX17049_ADDR);
    Wire.write(reg);
    Wire.write((value >> 8) & 0xFF);  // MSB
    Wire.write(value & 0xFF);          // LSB
    Wire.endTransmission();
}
