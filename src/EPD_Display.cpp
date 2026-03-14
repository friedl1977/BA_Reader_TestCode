#include "EPD_Display.h"

#if IMAGE_TEST_MODE
#include "image_data.h"
#include "image_data2.h"
#endif

// Include a basic font
#include <FreeSansBold24pt7b.h>

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

    // Hard reset the display before init
    pinMode(EPD_RST, OUTPUT);
    digitalWrite(EPD_RST, LOW);
    delay(20);
    digitalWrite(EPD_RST, HIGH);
    delay(20);

    // Check BUSY pin state before init
    pinMode(EPD_BUSY, INPUT);
    Serial.printlnf("EPD: BUSY pin (D22) before init: %d (0=ready, 1=busy)", digitalRead(EPD_BUSY));

    // Initialize display with 20ms reset pulse
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

        display.setTextColor(GxEPD_BLACK);
        display.setFont(&FreeSansBold24pt7b);

        int16_t tbx, tby;
        uint16_t tbw, tbh;

        // --- TOP: "Hello World! (TOP)" ---
        const char* topText = "Hello World! (TOP)";
        display.getTextBounds(topText, 0, 0, &tbx, &tby, &tbw, &tbh);
        int16_t topX = (display.width() - tbw) / 2 - tbx;
        int16_t topY = 50 - tby;
        display.setCursor(topX, topY);
        display.print(topText);

        // --- BOTTOM: "Hello World! (BOTTOM)" ---
        const char* bottomText = "Hello World! (BOTTOM)";
        display.getTextBounds(bottomText, 0, 0, &tbx, &tby, &tbw, &tbh);
        int16_t bottomX = (display.width() - tbw) / 2 - tbx;
        int16_t bottomY = display.height() - 50;
        display.setCursor(bottomX, bottomY);
        display.print(bottomText);

        // --- CENTER: Display info ---
        display.setFont();
        display.setTextSize(2);
        char info[64];
        snprintf(info, sizeof(info), "%dx%d pixels", display.width(), display.height());
        display.getTextBounds(info, 0, 0, &tbx, &tby, &tbw, &tbh);
        display.setCursor((display.width() - tbw) / 2, display.height() / 2);
        display.print(info);

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
