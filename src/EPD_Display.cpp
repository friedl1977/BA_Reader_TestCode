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

void EPD_Display::showHelloWorld() {
    Serial.println("EPD: showHelloWorld()");
    Serial.printlnf("EPD: BUSY pin before draw: %d", digitalRead(EPD_BUSY));

    display.setRotation(0);
    display.setFullWindow();

    Serial.println("EPD: calling firstPage()...");

    // Monitor BUSY pin for 2 seconds before firstPage to see if it changes
    int lastBusy = digitalRead(EPD_BUSY);
    unsigned long monStart = millis();
    while (millis() - monStart < 2000) {
        int b = digitalRead(EPD_BUSY);
        if (b != lastBusy) {
            Serial.printlnf("EPD: BUSY changed to %d at %lums", b, millis() - monStart);
            lastBusy = b;
        }
    }
    Serial.printlnf("EPD: BUSY before firstPage: %d", digitalRead(EPD_BUSY));

    display.firstPage();
    Serial.printlnf("EPD: firstPage() returned, BUSY now: %d", digitalRead(EPD_BUSY));

    // Monitor BUSY for 5 seconds after firstPage
    lastBusy = digitalRead(EPD_BUSY);
    monStart = millis();
    while (millis() - monStart < 5000) {
        int b = digitalRead(EPD_BUSY);
        if (b != lastBusy) {
            Serial.printlnf("EPD: BUSY changed to %d at %lums after firstPage", b, millis() - monStart);
            lastBusy = b;
        }
    }
    Serial.println("EPD: starting draw loop");
    do {
        display.fillScreen(GxEPD_WHITE);

        int cx = display.width() / 2;   // 100
        int cy = display.height() / 2;  // 100

        // Outer face circle
        display.drawCircle(cx, cy, 80, GxEPD_BLACK);
        display.drawCircle(cx, cy, 79, GxEPD_BLACK);

        // Eyes
        display.fillCircle(cx - 25, cy - 25, 8, GxEPD_BLACK);
        display.fillCircle(cx + 25, cy - 25, 8, GxEPD_BLACK);

        // Smile - arc using horizontal lines
        for (int angle = 20; angle <= 160; angle += 2) {
            float rad = angle * 3.14159f / 180.0f;
            int x = cx + (int)(45 * cos(rad));
            int y = cy + (int)(45 * sin(rad));
            display.fillCircle(x, y, 3, GxEPD_BLACK);
        }

        // "IT WORKS!" text below face
        display.setFont();
        display.setTextSize(2);
        display.setTextColor(GxEPD_BLACK);
        display.setCursor(44, 185);
        display.print("IT WORKS!");

    } while (display.nextPage());

    Serial.println("EPD: Hello World displayed successfully");
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
