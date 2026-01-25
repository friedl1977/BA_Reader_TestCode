#include "RFID.h"

// PE42412A-X Truth Table (LS=0)
// Antenna:  V4 V3 V2 V1
// RF1:       0  0  0  0
// RF2:       1  0  0  0
// RF3:       0  1  0  0
// RF4:       1  1  0  0
// RF5:       0  0  1  0
// RF6:       1  0  1  0
// RF7:       0  1  1  0
// RF8:       1  1  1  0
// RF9:       0  0  0  1
// RF10:      1  0  0  1
// RF11:      0  1  0  1
// RF12:      1  1  0  1
// ALL OFF:   0  0  1  1

RFID& RFID::instance() {
    static RFID _instance;
    return _instance;
}

bool RFID::begin() {
    // Setup pins
    pinMode(RF_V1, OUTPUT);
    pinMode(RF_V2, OUTPUT);
    pinMode(RF_V3, OUTPUT);
    pinMode(RF_V4, OUTPUT);
    pinMode(PN532_RST, OUTPUT);

    // Set antenna pins based on TEST_ANTENNA
    int v1 = 0, v2 = 0, v3 = 0, v4 = 0;
    switch (TEST_ANTENNA) {
        case 1:  v1=0; v2=0; v3=0; v4=0; break;
        case 2:  v1=0; v2=0; v3=0; v4=1; break;
        case 3:  v1=0; v2=0; v3=1; v4=0; break;
        case 4:  v1=0; v2=0; v3=1; v4=1; break;
        case 5:  v1=0; v2=1; v3=0; v4=0; break;
        case 6:  v1=0; v2=1; v3=0; v4=1; break;
        case 7:  v1=0; v2=1; v3=1; v4=0; break;
        case 8:  v1=0; v2=1; v3=1; v4=1; break;
        case 9:  v1=1; v2=0; v3=0; v4=0; break;
        case 10: v1=1; v2=0; v3=0; v4=1; break;
        case 11: v1=1; v2=0; v3=1; v4=0; break;
        case 12: v1=1; v2=0; v3=1; v4=1; break;
        default: v1=1; v2=1; v3=0; v4=0; break; // ALL OFF
    }

    digitalWrite(RF_V1, v1);
    digitalWrite(RF_V2, v2);
    digitalWrite(RF_V3, v3);
    digitalWrite(RF_V4, v4);

    if (TEST_ANTENNA == 0) {
        Serial.println("Antenna: ALL OFF");
    } else {
        Serial.printlnf("Antenna %d: V1=%d V2=%d V3=%d V4=%d", TEST_ANTENNA, v1, v2, v3, v4);
    }

    // Reset PN532
    digitalWrite(PN532_RST, LOW);
    delay(10);
    digitalWrite(PN532_RST, HIGH);
    delay(50);

    // Init PN532 (Wire.begin() called in main.cpp)
    _nfc = new DFRobot_PN532_IIC(PN532_IRQ, 0);

    if (!_nfc->begin()) {
        Serial.println("PN532 init FAILED");
        _initialized = false;
        return false;
    }

    Serial.println("PN532 OK - Scanning...");
    _initialized = true;
    return true;
}

bool RFID::scan(uint8_t* uid) {
    if (!_initialized) return false;

    if (_nfc->scan()) {
        memcpy(uid, _nfc->nfcUid, 4);
        return true;
    }
    return false;
}
