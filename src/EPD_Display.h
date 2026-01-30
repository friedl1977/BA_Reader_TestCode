#ifndef __EPD_DISPLAY_H
#define __EPD_DISPLAY_H

#include "Particle.h"

// EPD Pin definitions for BA Reader board
#define EPD_CS    D8
#define EPD_DC    D4
#define EPD_RST   D25
#define EPD_BUSY  D22

// Enable GxEPD2_GFX base class
#define ENABLE_GxEPD2_GFX 1

// =====================================================
// Display Selection - uncomment ONE section below:
// =====================================================

// --- Option 1: 4.2" 3-color display (for testing) ---
//#define USE_42_INCH_3C

// --- Option 2: 13.3" B&W display ---
#define USE_133_INCH_BW

// =====================================================
// Auto-configuration based on selection
// =====================================================
#define MAX_DISPLAY_BUFFER_SIZE 65536ul

#if defined(USE_42_INCH_3C)
    #include <GxEPD2_3C.h>
    #define GxEPD2_DRIVER_CLASS GxEPD2_420c  // GDEW042Z15 400x300, UC8176 (Waveshare)
    #define MAX_HEIGHT(EPD) (EPD::HEIGHT <= (MAX_DISPLAY_BUFFER_SIZE / 2) / (EPD::WIDTH / 8) ? EPD::HEIGHT : (MAX_DISPLAY_BUFFER_SIZE / 2) / (EPD::WIDTH / 8))
    typedef GxEPD2_3C<GxEPD2_DRIVER_CLASS, MAX_HEIGHT(GxEPD2_DRIVER_CLASS)> EPD_Display_t;

#elif defined(USE_133_INCH_BW)
    #include <GxEPD2_BW.h>
    #define GxEPD2_DRIVER_CLASS GxEPD2_1330_GDEM133T91  // 960x680, SSD1677
    #define MAX_HEIGHT(EPD) (EPD::HEIGHT <= MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8) ? EPD::HEIGHT : MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8))
    typedef GxEPD2_BW<GxEPD2_DRIVER_CLASS, MAX_HEIGHT(GxEPD2_DRIVER_CLASS)> EPD_Display_t;

#else
    #error "Please define USE_42_INCH_3C or USE_133_INCH_BW in EPD_Display.h"
#endif

/**
 * EPD Display singleton class for 13.3" e-paper display
 */
class EPD_Display {
public:
    static EPD_Display &instance();

    void begin();
    void showHelloWorld();
    void hibernate();

    // Access to underlying display for advanced usage
    EPD_Display_t& getDisplay() { return display; }

private:
    EPD_Display();
    virtual ~EPD_Display();
    
    EPD_Display(const EPD_Display&) = delete;
    EPD_Display& operator=(const EPD_Display&) = delete;

    static EPD_Display *_instance;
    EPD_Display_t display;
};

#endif /* __EPD_DISPLAY_H */
