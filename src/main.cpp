#include "Particle.h"
#include "RFID.h"
#include "Buzzer.h"
#include "Battery.h"

// =====================================================
// Enable/disable Particle cloud publishing
// =====================================================
#define ENABLE_CLOUD_PUBLISH  1

SYSTEM_MODE(SEMI_AUTOMATIC);

unsigned long lastBattRead = 0;
unsigned long lastPublish = 0;

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("BA Reader Test");

    Buzzer::instance().init();
    Buzzer::instance().playSuccessTone();

    Wire.begin();  // Init I2C first

    RFID::instance().begin();
    Battery::instance().begin();

#if ENABLE_CLOUD_PUBLISH
    Particle.connect();
    Serial.println("Connecting to cloud...");
#endif
}

void loop() {
#if ENABLE_CLOUD_PUBLISH
    Particle.process();
#endif

    // RFID scanning
    uint8_t uid[4];
    if (RFID::instance().scan(uid)) {
        Serial.printlnf("CARD: %02X%02X%02X%02X", uid[0], uid[1], uid[2], uid[3]);
        Buzzer::instance().playSuccessTone();
        delay(1000);
    }

    // Battery to serial every 5 seconds
    if (millis() - lastBattRead >= 5000) {
        float soc = Battery::instance().getSoC();
        float voltage = Battery::instance().getVoltage();
        Serial.printlnf("Battery: %.1f%% (%.2fV)", soc, voltage);
        lastBattRead = millis();
    }

#if ENABLE_CLOUD_PUBLISH
    // Publish to cloud every 30 seconds
    if (Particle.connected() && millis() - lastPublish >= 60000) {
        float soc = Battery::instance().getSoC();
        float voltage = Battery::instance().getVoltage();
        uint16_t raw = Battery::instance().getRawVoltage();
        char data[96];
        snprintf(data, sizeof(data), "{\"soc\":%.1f,\"voltage\":%.2f,\"raw\":%u}", soc, voltage, raw);
        Particle.publish("battery", data, PRIVATE);
        Serial.printlnf("Published: %s", data);
        lastPublish = millis();
    }
#endif
}
