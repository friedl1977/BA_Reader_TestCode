#include "Charger.h"

Charger& Charger::instance() {
    static Charger _instance;
    return _instance;
}

bool Charger::begin() {
    Wire.beginTransmission(MP2672A_ADDR);
    if (Wire.endTransmission() != 0) {
        Serial.println("MP2672A not found at 0x4B");
        _initialized = false;
        return false;
    }

    Serial.println("MP2672A found");
    _initialized = true;
    readAllRegisters();
    return true;
}

void Charger::readAllRegisters() {
    if (!_initialized) return;

    Serial.println("--- MP2672A Registers ---");

    // REG00: Battery voltage, charge config
    uint8_t reg00 = readReg(REG00_VBATT);
    Serial.printlnf("REG00 (VBATT/Config): 0x%02X = %s", reg00, String(reg00, BIN).c_str());
    uint8_t vbatt = (reg00 >> 5) & 0x07;
    const char* vbattStr[] = {"8.3V", "8.4V", "8.5V", "8.6V", "8.7V", "8.8V", "8.9V", "8.2V"};
    Serial.printlnf("  VBATT_REG: %s", vbattStr[vbatt]);

    // REG01: Cell balance, charge current
    uint8_t reg01 = readReg(REG01_BALANCE);
    Serial.printlnf("REG01 (Balance/Current): 0x%02X = %s", reg01, String(reg01, BIN).c_str());

    // REG02: Timer settings
    uint8_t reg02 = readReg(REG02_TIMER);
    Serial.printlnf("REG02 (Timer): 0x%02X = %s", reg02, String(reg02, BIN).c_str());

    // REG03: Status (read-only)
    uint8_t reg03 = readReg(REG03_STATUS);
    Serial.printlnf("REG03 (Status): 0x%02X = %s", reg03, String(reg03, BIN).c_str());
    Serial.printlnf("  CHG_STAT[1:0]: %d (0=Not charging, 1=Pre-charge, 2=Fast charge, 3=Done)", (reg03 >> 4) & 0x03);
    Serial.printlnf("  CELL_BAL: %d", (reg03 >> 3) & 0x01);
    Serial.printlnf("  VIN_STAT: %d", (reg03 >> 2) & 0x01);
    Serial.printlnf("  THERM_STAT: %d", (reg03 >> 1) & 0x01);
    Serial.printlnf("  VSYS_STAT: %d", reg03 & 0x01);

    // REG04: Fault (read-only)
    uint8_t reg04 = readReg(REG04_FAULT);
    Serial.printlnf("REG04 (Fault): 0x%02X = %s", reg04, String(reg04, BIN).c_str());
    Serial.printlnf("  WD_FAULT: %d", (reg04 >> 7) & 0x01);
    Serial.printlnf("  VIN_OVP: %d", (reg04 >> 6) & 0x01);
    Serial.printlnf("  THERM_SD: %d", (reg04 >> 5) & 0x01);
    Serial.printlnf("  BAT_FAULT: %d", (reg04 >> 4) & 0x01);
    Serial.printlnf("  CHG_TMR_FAULT: %d", (reg04 >> 3) & 0x01);

    Serial.println("-------------------------");
}

uint8_t Charger::readReg(uint8_t reg) {
    Wire.beginTransmission(MP2672A_ADDR);
    Wire.write(reg);
    Wire.endTransmission(false);

    Wire.requestFrom(MP2672A_ADDR, 1);
    if (Wire.available()) {
        return Wire.read();
    }
    return 0xFF;
}
