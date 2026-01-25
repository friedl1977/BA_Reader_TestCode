#include "I2C_Utils.h"

void I2C_Utils::scanBus() {
    Log.info("I2C bus scan starting...");

    uint8_t count = 0;
    for (uint8_t addr = 1; addr < 127; addr++) {
        if (devicePresent(addr)) {
            Log.info("  Device found at 0x%02X", addr);
            count++;
        }
    }

    if (count == 0) {
        Log.warn("  No I2C devices found");
    } else {
        Log.info("I2C scan complete: %d device(s) found", count);
    }
}

bool I2C_Utils::devicePresent(uint8_t address) {
    Wire.beginTransmission(address);
    return (Wire.endTransmission() == 0);
}
