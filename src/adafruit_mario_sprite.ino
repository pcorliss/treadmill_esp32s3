/**************************************************************************
  This is a library for several Adafruit displays based on ST77* drivers.

  Works with the Adafruit ESP32-S2 TFT Feather
    ----> http://www.adafruit.com/products/5300

  Check out the links above for our tutorials and wiring diagrams.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 **************************************************************************/

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>
#include "mario.h"

// Use dedicated hardware SPI pins
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);


void setup(void) {
  Serial.begin(9600);
  Serial.print(F("Hello! Feather TFT Test"));



  // turn on backlite
  pinMode(TFT_BACKLITE, OUTPUT);
  digitalWrite(TFT_BACKLITE, HIGH);

  // turn on the TFT / I2C power supply
  pinMode(TFT_I2C_POWER, OUTPUT);
  digitalWrite(TFT_I2C_POWER, HIGH);
  delay(10);

  // initialize TFT
  tft.init(135, 240); // Init ST7789 240x135
  tft.setRotation(3);
  tft.fillScreen(ST77XX_BLACK);

  Serial.println(F("Initialized"));

  Serial.print(F("TFT_RST:"));
  Serial.println(TFT_RST);
  Serial.print(F("TFT_CS:"));
  Serial.println(TFT_CS);
  Serial.print(F("TFT_DC:"));
  Serial.println(TFT_DC);

// TFT_RST:41
// TFT_CS:42
// TFT_DC:40

// TFT_WIDTH 240
//TFT_HEIGHT 135

  Serial.println("Setup done");
  delay(1000);
}

void loop() {
  Serial.println("Looping...");
  testSprite();
  delay(500);
}

void testSprite()
{
  Serial.println("Rendering Mario...");
  int prevMario = 6;
  int prevCape = 3;
  for (int mario_i = 0, cape = 0; true; mario_i = (mario_i + 1) % 3, cape = (cape + 1) % 4)
  {
    int mario = mario_i + 4;
    tft.drawRGBBitmap(10, 9, epd_bitmap_allArray[cape], epd_bitmap_alphaallArray[prevCape], 16, 16);
    tft.drawRGBBitmap(0, 0, epd_bitmap_allArray[mario], epd_bitmap_alphaallArray[prevMario], 16, 32);
    tft.drawRGBBitmap(10, 9, epd_bitmap_allArray[cape], epd_bitmap_alphaallArray[cape], 16, 16);
    tft.drawRGBBitmap(0, 0, epd_bitmap_allArray[mario], epd_bitmap_alphaallArray[mario], 16, 32);
    prevMario = mario;
    prevCape = cape;
    delay(200);
  }
}
