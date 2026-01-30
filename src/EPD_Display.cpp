#include "EPD_Display.h"

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
    logr.info("Initializing EPD display...");

    SPI.begin();

    // Initialize display with 2ms reset pulse for Waveshare boards
    display.init(115200, true, 2, false);

    logr.info("EPD initialized. Resolution: %dx%d", display.width(), display.height());
}

void EPD_Display::showHelloWorld() {
    logr.info("Displaying Hello World...");

    display.setRotation(0);
    display.setFullWindow();

    display.firstPage();
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

    logr.info("Hello World displayed successfully");
}

void EPD_Display::hibernate() {
    logr.info("EPD entering hibernate mode");
    display.hibernate();
}
