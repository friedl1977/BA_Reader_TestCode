#include "EPD_Display.h"
#include <math.h>

#if IMAGE_TEST_MODE
#include "image_data.h"
#include "image_data2.h"
#endif


static Logger logr("app.epd");

EPD_Display *EPD_Display::_instance = nullptr;

EPD_Display &EPD_Display::instance() {
    if (!_instance) {
        _instance = new EPD_Display();
    }
    return *_instance;
}

EPD_Display::EPD_Display()
    : display(GxEPD2_DRIVER_CLASS(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY)) {
}

EPD_Display::~EPD_Display() {
}

void EPD_Display::begin() {
    Serial.println("EPD: begin()");

    SPI.begin();

    // Monitor BUSY during manual RST toggle
    pinMode(EPD_BUSY, INPUT);
    pinMode(EPD_RST, OUTPUT);

    Serial.printlnf("EPD: BUSY before RST LOW: %d", digitalRead(EPD_BUSY));
    digitalWrite(EPD_RST, LOW);
    delay(20);
    Serial.printlnf("EPD: BUSY during RST LOW: %d", digitalRead(EPD_BUSY));
    digitalWrite(EPD_RST, HIGH);
    delay(20);
    Serial.printlnf("EPD: BUSY after RST HIGH: %d", digitalRead(EPD_BUSY));

    // Monitor BUSY for 500ms after reset to catch any transition
    int lastBusy = digitalRead(EPD_BUSY);
    unsigned long t = millis();
    while (millis() - t < 500) {
        int b = digitalRead(EPD_BUSY);
        if (b != lastBusy) {
            Serial.printlnf("EPD: BUSY changed to %d at %lums after RST", b, millis() - t);
            lastBusy = b;
        }
    }
    Serial.printlnf("EPD: BUSY pin (D22) before init: %d (0=ready, 1=busy)", digitalRead(EPD_BUSY));

    display.init(115200, true, 20, false);

    Serial.printlnf("EPD: BUSY pin after init: %d", digitalRead(EPD_BUSY));
    Serial.printlnf("EPD: Resolution: %dx%d", display.width(), display.height());
}


void EPD_Display::showWaiting() {
    display.setRotation(0);
    display.setFullWindow();
    display.firstPage();
    do {
        display.fillScreen(GxEPD_WHITE);
        display.setFont();
        display.setTextSize(3);
        display.setTextColor(GxEPD_BLACK);
        display.setCursor(28, 90);
        display.print("Waiting...");
    } while (display.nextPage());
}

void EPD_Display::showCardScanned(const char* uid) {
    int cx = display.width() / 2;   // 100
    int cy = display.height() / 4;  // 50 - top half

    display.setRotation(0);
    display.setFullWindow();
    display.firstPage();
    do {
        display.fillScreen(GxEPD_WHITE);

        // Winking face outline
        display.drawCircle(cx, cy, 40, GxEPD_BLACK);
        display.drawCircle(cx, cy, 39, GxEPD_BLACK);

        // Left eye - open
        display.fillCircle(cx - 13, cy - 12, 5, GxEPD_BLACK);

        // Right eye - winking (closed, drawn as a line)
        display.drawLine(cx + 8, cy - 12, cx + 18, cy - 12, GxEPD_BLACK);
        display.drawLine(cx + 8, cy - 13, cx + 18, cy - 13, GxEPD_BLACK);

        // Smile
        for (int angle = 20; angle <= 160; angle += 3) {
            float rad = angle * 3.14159f / 180.0f;
            int x = cx + (int)(22 * cos(rad));
            int y = cy + (int)(22 * sin(rad));
            display.fillCircle(x, y, 2, GxEPD_BLACK);
        }

        // UID in bottom half
        display.setFont();
        display.setTextSize(2);
        display.setTextColor(GxEPD_BLACK);
        display.setCursor(10, 120);
        display.print("Card UID:");
        display.setCursor(10, 150);
        display.print(uid);

    } while (display.nextPage());
}

#if IMAGE_TEST_MODE
void EPD_Display::showCustomImage() {
    logr.info("Displaying custom image 1...");

    // Image generated for 960x680 from e-paper-display.com
    const int16_t imgWidth = 960;
    const int16_t imgHeight = 680;

    display.setRotation(0);
    display.setFullWindow();

    display.firstPage();
    do {
        display.fillScreen(GxEPD_WHITE);

        // Draw full screen - use drawInvertedBitmap to flip black/white
        display.drawInvertedBitmap(0, 0, gImage, imgWidth, imgHeight, GxEPD_BLACK);

    } while (display.nextPage());

    logr.info("Custom image 1 displayed (960x680, inverted)");
}

void EPD_Display::showCustomImage2() {
    logr.info("Displaying custom image 2...");

    // Image generated for 960x680 from e-paper-display.com
    const int16_t imgWidth = 960;
    const int16_t imgHeight = 680;

    display.setRotation(0);
    display.setFullWindow();

    display.firstPage();
    do {
        display.fillScreen(GxEPD_WHITE);

        // Draw full screen - use drawInvertedBitmap to flip black/white
        display.drawInvertedBitmap(0, 0, gImage2, imgWidth, imgHeight, GxEPD_BLACK);

    } while (display.nextPage());

    logr.info("Custom image 2 displayed (960x680, inverted)");
}

#endif // IMAGE_TEST_MODE

#if IMAGE_TEST_MODE
void EPD_Display::toggleImage() {
    currentImage = !currentImage;

    if (currentImage) {
        showCustomImage2();
    } else {
        showCustomImage();
    }
}
#endif // IMAGE_TEST_MODE

void EPD_Display::hibernate() {
    logr.info("EPD entering hibernate mode");
    display.hibernate();
}
