#include "Battery.h"

// MAX17049 Register addresses
#define REG_VCELL   0x02  // Battery voltage
#define REG_SOC     0x04  // State of charge
#define REG_MODE    0x06  // Mode register
#define REG_VERSION 0x08  // IC version
#define REG_CONFIG  0x0C  // Config (RCOMP, Sleep, Alert)
#define REG_VALRT   0x14  // Voltage alert thresholds
#define REG_CRATE   0x16  // Charge/discharge rate
#define REG_VRESET  0x18  // VRESET/ID
#define REG_STATUS  0x1A  // Status register
#define REG_CMD     0xFE  // Command register

Battery& Battery::instance() {
    static Battery _instance;
    return _instance;
}

bool Battery::begin() {
    pinMode(BATT_ALERT_PIN, INPUT);

    // Check if device responds
    Wire.beginTransmission(MAX17049_ADDR);
    if (Wire.endTransmission() != 0) {
        Serial.println("MAX17049 not found");
        _initialized = false;
        return false;
    }

    _initialized = true;

    // Issue Quick Start to recalculate SOC from current voltage
    quickStart();
    delay(200);  // Wait for recalculation

    // Read all registers after quick start
    Serial.println("--- MAX17049 Registers ---");
    Serial.printlnf("VCELL:   0x%04X", readReg(REG_VCELL));
    Serial.printlnf("SOC:     0x%04X (%.1f%%)", readReg(REG_SOC), (float)readReg(REG_SOC) / 256.0f);
    Serial.printlnf("MODE:    0x%04X", readReg(REG_MODE));
    Serial.printlnf("VERSION: 0x%04X", readReg(REG_VERSION));
    Serial.printlnf("CONFIG:  0x%04X (RCOMP=%d)", readReg(REG_CONFIG), readReg(REG_CONFIG) >> 8);
    Serial.println("--------------------------");

    return true;
}

float Battery::getSoC() {
    if (!_initialized) return -1;

    // Use voltage-based SOC for 2S Li-ion
    // Empty = 6.0V (3.0V/cell), Full = 8.4V (4.2V/cell)
    float voltage = getVoltage();
    float soc = (voltage - 6.0f) / (8.4f - 6.0f) * 100.0f;
    if (soc < 0.0f) soc = 0.0f;
    if (soc > 100.0f) soc = 100.0f;
    return soc;
}

float Battery::getVoltage() {
    if (!_initialized) return -1;

    uint16_t raw = readReg(REG_VCELL);
    // MAX17049 (2S): 156.25uV per LSB (datasheet value)
    float voltage = (float)raw * 0.00015625f;
    return voltage;
}

uint16_t Battery::getRawVoltage() {
    if (!_initialized) return 0;
    return readReg(REG_VCELL);
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

void Battery::quickStart() {
    // Quick Start: write 0x4000 to MODE register
    // Forces the chip to restart SOC calculation from current voltage
    writeReg(REG_MODE, 0x4000);
    Serial.println("MAX17049 Quick Start issued");
}
