#define ST7789_DRIVER // Full configuration option, define additional parameters below for this display

// Define the display dimensions
#define TFT_WIDTH 135
#define TFT_HEIGHT 240

// Define the pin connections for your ESP32 S2 Mini
#define TFT_CS 42  // Chip Select
#define TFT_RST 41 // Reset
#define TFT_DC 40  // Data/Command (A0)

// SPI pins
#define TFT_SCLK 36 // Clock (SCK)
#define TFT_MOSI 35 // Data out (SDA)
#define TFT_MISO -1 // Not used

// Optional: Define the backlight pin and set it to HIGH to turn on the backlight
#define TFT_BL 45 // LED back-light (not controlled by GPIO since connected to 3.3V)
#define TFT_BACKLIGHT TFT_BL

// #define LOAD_GLCD  // Font 1. Original Adafruit 8 pixel font needs ~1820 bytes in FLASH
// #define LOAD_FONT2 // Font 2. Small 16 pixel high font, needs ~3534 bytes in FLASH, 96 characters
// #define LOAD_FONT4 // Font 4. Medium 26 pixel high font, needs ~5848 bytes in FLASH, 96 characters
// #define LOAD_FONT6 // Font 6. Large 48 pixel font, needs ~2666 bytes in FLASH, only characters 1234567890:-.apm
// #define LOAD_FONT7 // Font 7. 7 segment 48 pixel font, needs ~2438 bytes in FLASH, only characters 1234567890:-.
// #define LOAD_FONT8 // Font 8. Large 75 pixel font needs ~3256 bytes in FLASH, only characters 1234567890:-.
// // #define LOAD_FONT8N // Font 8. Alternative to Font 8 above, slightly narrower, so 3 digits fit a 160 pixel TFT
// #define LOAD_GFXFF // FreeFonts. Include access to the 48 Adafruit_GFX free fonts FF1 to FF48 and custom fonts

// #define SMOOTH_FONT

#define SPI_FREQUENCY 40000000
#define USE_HSPI_PORT