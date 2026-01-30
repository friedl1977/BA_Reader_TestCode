#include "Particle.h"
#include "RFID.h"
#include "Buzzer.h"
#include "Battery.h"
#include "Charger.h"
#include "EPD_Display.h"
#include "Buttons.h"

using namespace std::chrono_literals;

// =====================================================
// Enable/disable Particle cloud publishing
// =====================================================
#define ENABLE_CLOUD_PUBLISH  1

// =====================================================
// Enable/disable EPD display test
// =====================================================
#define ENABLE_EPD_TEST  1

// =====================================================
// Timing intervals using chrono literals
// =====================================================
constexpr auto BATTERY_READ_INTERVAL = 5s;
constexpr auto CLOUD_PUBLISH_INTERVAL = 5min;

// =====================================================
// Low battery threshold for hibernate
// =====================================================
constexpr float LOW_BATTERY_THRESHOLD = 5.0f;  // Hibernate at 5% SoC

// =====================================================
// Charger status pin (MP2672 ACOK)
// ACOK is LOW when external power is present
// =====================================================
#define CHARGER_ACOK_PIN  D20

SYSTEM_MODE(SEMI_AUTOMATIC);

unsigned long lastBattRead = 0;
unsigned long lastPublish = 0;

// Forward declarations
void enterHibernate();
bool isCharging();

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("BA Reader Test");

    Buzzer::instance().init();
    Buzzer::instance().playSuccessTone();

    Wire.begin();  // Init I2C first

    RFID::instance().begin();
    Battery::instance().begin();
    Buttons::instance().begin();
    Charger::instance().begin();  // Requires Host-Control mode (CV pin to VCC)

    // Configure charger status pin
    pinMode(CHARGER_ACOK_PIN, INPUT);

#if ENABLE_EPD_TEST
    Serial.println("Initializing EPD display...");
    EPD_Display::instance().begin();
    Serial.println("Displaying Hello World...");
    EPD_Display::instance().showHelloWorld();
    EPD_Display::instance().hibernate();
    Serial.println("EPD test complete");
#endif

#if ENABLE_CLOUD_PUBLISH
    Particle.connect();
    Serial.println("Connecting to cloud...");
#endif
}

void loop() {
#if ENABLE_CLOUD_PUBLISH
    Particle.process();
#endif

    // Check buttons
    Buttons::instance().update();

    // RFID scanning
    uint8_t uid[4];
    if (RFID::instance().scan(uid)) {
        Serial.printlnf("CARD: %02X%02X%02X%02X", uid[0], uid[1], uid[2], uid[3]);
        Buzzer::instance().playSuccessTone();
        delay(1000);
    }

    // Battery to serial every 5 seconds
    if (millis() - lastBattRead >= BATTERY_READ_INTERVAL.count() * 1000) {
        float soc = Battery::instance().getSoC();
        float voltage = Battery::instance().getVoltage();
        bool charging = isCharging();
        bool thermalLimit = Charger::instance().isThermalRegulation();

        Serial.printlnf("Battery: %.1f%% (%.2fV) %s%s", soc, voltage,
            charging ? "[Charging]" : "[On Battery]",
            thermalLimit ? " [Thermal Limit]" : "");

        // Show charger status if available
        if (Charger::instance().isAvailable() && charging) {
            Serial.printlnf("  Charger: %s", Charger::instance().getChargeStatusString());
        }

        lastBattRead = millis();

        // Check for low battery and enter hibernate (only if not charging)
        if (soc <= LOW_BATTERY_THRESHOLD && soc > 0 && !charging) {
            enterHibernate();
        }
    }

#if ENABLE_CLOUD_PUBLISH
    // Publish to cloud every 5 minutes
    if (Particle.connected() && millis() - lastPublish >= std::chrono::milliseconds(CLOUD_PUBLISH_INTERVAL).count()) {
        float soc = Battery::instance().getSoC();
        float voltage = Battery::instance().getVoltage();
        uint16_t raw = Battery::instance().getRawVoltage();
        bool charging = isCharging();
        bool thermalLimit = Charger::instance().isThermalRegulation();
        char data[192];
        snprintf(data, sizeof(data),
            "{\"soc\":%.1f,\"voltage\":%.2f,\"raw\":%u,\"charging\":%s,\"thermalLimit\":%s}",
            soc, voltage, raw,
            charging ? "true" : "false",
            thermalLimit ? "true" : "false");
        Particle.publish("battery", data, PRIVATE);
        Serial.printlnf("Published: %s", data);
        lastPublish = millis();
    }
#endif
}

void enterHibernate() {
    float soc = Battery::instance().getSoC();
    float voltage = Battery::instance().getVoltage();

    Serial.printlnf("Low battery (%.1f%%) - entering hibernate mode", soc);

#if ENABLE_CLOUD_PUBLISH
    // Publish final status before sleeping
    if (Particle.connected()) {
        char data[128];
        snprintf(data, sizeof(data),
            "{\"soc\":%.1f,\"voltage\":%.2f,\"status\":\"hibernating\"}",
            soc, voltage);
        Particle.publish("battery", data, PRIVATE);
        Serial.println("Published sleep notification");

        // Wait for publish to complete
        delay(2000);
    }
#endif

    // Play sleep tone
    Buzzer::instance().playSleepTone();
    delay(500);

    Serial.println("Going to hibernate - press Button 3 (A7) to wake");
    Serial.flush();

    // Configure hibernate with Button 3 (A7) as wake source
    // Button is active LOW with pull-up, so wake on FALLING edge
    SystemSleepConfiguration config;
    config.mode(SystemSleepMode::HIBERNATE)
          .gpio(BUTTON_3_PIN, FALLING);

    System.sleep(config);

    // Device will reset after waking from hibernate
    // This code won't be reached - device restarts from setup()
}

bool isCharging() {
    // MP2672 ACOK pin is LOW when external power is present
    return digitalRead(CHARGER_ACOK_PIN) == LOW;
}
