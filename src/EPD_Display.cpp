#include "EPD_Display.h"
#include "image_data.h"

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

void EPD_Display::showCustomImage() {
    logr.info("Displaying custom image...");

    // For monochrome EPD, images are typically bit-packed
    // 240,000 bytes * 8 bits = 1,920,000 pixels
    // Common EPD sizes that match:
    // - 800x2400 = 1,920,000 pixels (too tall)
    // - 960x2000 = 1,920,000 pixels (too tall)
    // - 1600x1200 = 1,920,000 pixels (too wide)

    // Most likely: Display width (960) x some height
    // 1,920,000 / 960 = 2000 pixels tall - too tall

    // OR the image was generated for a different display size
    // Try 800x300 pixel image stored with bit packing:
    // 800 / 8 = 100 bytes per row
    // 300 rows * 100 bytes = 30,000 bytes (not 240,000)

    // So 240,000 bytes must be 1 byte per pixel format
    // Try 800x300 as raw bytes
    const int16_t imgWidth = 800;
    const int16_t imgHeight = 300;

    display.setRotation(0);
    display.setFullWindow();

    display.firstPage();
    do {
        display.fillScreen(GxEPD_WHITE);

        // Center on display
        int16_t x = (display.width() - imgWidth) / 2;
        int16_t y = (display.height() - imgHeight) / 2;

        // Try drawing the image as a grayscale/raw byte array
        // Since drawBitmap expects bit-packed, let's use drawGrayscaleBitmap or drawInvertedBitmap
        display.drawInvertedBitmap(x, y, gImage, imgWidth, imgHeight, GxEPD_BLACK);

    } while (display.nextPage());

    logr.info("Custom image displayed (800x300, inverted)");
}

void EPD_Display::hibernate() {
    logr.info("EPD entering hibernate mode");
    display.hibernate();
}
