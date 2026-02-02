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

    // The e-paper-display.com converter outputs RGB888 format even for B&W
    // 960x680 pixels × 3 bytes (RGB) = 1,958,400 bytes (but we have 240,000)
    // OR it's outputting grayscale: 960x680 / 3 = 652,800 / 3 = 217,600 pixels
    // Actually: 800 × 300 × 1 byte = 240,000 bytes!

    // The converter likely resized or cropped. Let's try 800×300:
    const int16_t imgWidth = 800;
    const int16_t imgHeight = 300;

    display.setRotation(0);
    display.setFullWindow();

    display.firstPage();
    do {
        display.fillScreen(GxEPD_WHITE);

        // Center the image
        int16_t x = (display.width() - imgWidth) / 2;
        int16_t y = (display.height() - imgHeight) / 2;

        // The data is likely 1 byte per pixel (0x00=black, 0xFF=white)
        // We need to convert it to bitmap format for GxEPD2
        for (int16_t row = 0; row < imgHeight; row++) {
            for (int16_t col = 0; col < imgWidth; col++) {
                uint8_t pixel = gImage[row * imgWidth + col];
                // Draw white pixels (0xFF or high values) as white, black as black
                if (pixel < 128) {
                    display.drawPixel(x + col, y + row, GxEPD_BLACK);
                }
            }
        }

    } while (display.nextPage());

    logr.info("Custom image displayed (800x300, byte-per-pixel)");
}

void EPD_Display::hibernate() {
    logr.info("EPD entering hibernate mode");
    display.hibernate();
}
