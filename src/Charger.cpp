#include "Charger.h"

Charger& Charger::instance() {
    static Charger _instance;
    return _instance;
}

bool Charger::begin() {
    Wire.beginTransmission(MP2672A_ADDR);
    if (Wire.endTransmission() != 0) {
        Serial.println("MP2672A not found at 0x4B (Host-Control mode required)");
        _initialized = false;
        return false;
    }

    Serial.println("MP2672A found");
    _initialized = true;
    readAllRegisters();
    return true;
}

// =====================================================
// Status register (REG03) readings
// =====================================================

ChargeStatus Charger::getChargeStatus() {
    if (!_initialized) return ChargeStatus::NOT_CHARGING;
    uint8_t reg = readReg(REG03_STATUS);
    return static_cast<ChargeStatus>((reg >> 4) & 0x03);
}

const char* Charger::getChargeStatusString() {
    switch (getChargeStatus()) {
        case ChargeStatus::NOT_CHARGING: return "Not Charging";
        case ChargeStatus::PRE_CHARGE:   return "Pre-charge";
        case ChargeStatus::FAST_CHARGE:  return "Fast Charge";
        case ChargeStatus::CHARGE_DONE:  return "Charge Done";
        default: return "Unknown";
    }
}

bool Charger::isCellBalancing() {
    if (!_initialized) return false;
    uint8_t reg = readReg(REG03_STATUS);
    return (reg >> 3) & 0x01;
}

bool Charger::isInputPowerGood() {
    if (!_initialized) return false;
    uint8_t reg = readReg(REG03_STATUS);
    return (reg >> 2) & 0x01;
}

bool Charger::isThermalRegulation() {
    if (!_initialized) return false;
    uint8_t reg = readReg(REG03_STATUS);
    // THERM_STAT bit 1: 1 = thermal regulation active (charge current reduced)
    return (reg >> 1) & 0x01;
}

bool Charger::isSystemMinVoltage() {
    if (!_initialized) return false;
    uint8_t reg = readReg(REG03_STATUS);
    return reg & 0x01;
}

// =====================================================
// Fault register (REG04) readings
// =====================================================

bool Charger::hasWatchdogFault() {
    if (!_initialized) return false;
    uint8_t reg = readReg(REG04_FAULT);
    return (reg >> 7) & 0x01;
}

bool Charger::hasInputOVP() {
    if (!_initialized) return false;
    uint8_t reg = readReg(REG04_FAULT);
    return (reg >> 6) & 0x01;
}

bool Charger::hasThermalShutdown() {
    if (!_initialized) return false;
    uint8_t reg = readReg(REG04_FAULT);
    return (reg >> 5) & 0x01;
}

bool Charger::hasBatteryFault() {
    if (!_initialized) return false;
    uint8_t reg = readReg(REG04_FAULT);
    return (reg >> 4) & 0x01;
}

bool Charger::hasChargeTimerFault() {
    if (!_initialized) return false;
    uint8_t reg = readReg(REG04_FAULT);
    return (reg >> 3) & 0x01;
}

bool Charger::hasAnyFault() {
    if (!_initialized) return false;
    uint8_t reg = readReg(REG04_FAULT);
    // Check bits 7:3 for any fault
    return (reg & 0xF8) != 0;
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
