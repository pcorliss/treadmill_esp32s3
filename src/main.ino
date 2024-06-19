#include <TFT_eSPI.h> // Hardware-specific library
#include <SPI.h>
#include <mario.h>

TFT_eSPI tft = TFT_eSPI(); // Invoke custom library
TFT_eSprite mario = TFT_eSprite(&tft);
TFT_eSprite background = TFT_eSprite(&tft);

void setup(void)
{
  Serial.begin(9600);
  Serial.print("Hello! ST7735 TFT Test");

  // turn on backlite
  pinMode(TFT_BACKLITE, OUTPUT);
  digitalWrite(TFT_BACKLITE, HIGH);

  // turn on the TFT / I2C power supply
  pinMode(TFT_I2C_POWER, OUTPUT);
  digitalWrite(TFT_I2C_POWER, HIGH);
  delay(10);

  // Use this initializer if you're using a 1.8" TFT
  tft.init(); // initialize a ST7735S chip
  tft.setRotation(1);
  tft.setSwapBytes(true);

  background.createSprite(TFT_HEIGHT, TFT_WIDTH);
  background.setSwapBytes(tft.getSwapBytes());
  background.fillSprite(TFT_DARKGREY);

  // tft.fillScreen(TFT_DARKGREY);

  Serial.println("Initialized");
  mario.setSwapBytes(tft.getSwapBytes());
  mario.createSprite(16, 32);
  // mario.pushImage(0, 0, 16, 32, (uint16_t *)epd_bitmap_allArray[4], 0x0000);
  // mario.pushImage(0, 0, 16, 32, (uint16_t *)epd_bitmap_allArray[4]);
}

void loop(void)
{
  for (int x = 0, mario_idx = 0; x < TFT_HEIGHT - 16; x++, mario_idx++)
  {
    background.fillSprite(TFT_DARKGREY);
    // mario.pushImage(0, 0, 16, 32, (uint16_t *)epd_bitmap_allArray[mario_idx % 3 + 4]);
    mario.pushImage(0, 0, 16, 32, (uint16_t *)epd_bitmap_allArray[mario_idx % 3 + 4]);
    mario.pushToSprite(&background, x, 0, TFT_BLACK);

    background.pushSprite(0, 0);

    // mario.pushImage(x, 0, 16, 32, (uint16_t *)epd_bitmap_allArray[mario_idx % 3 + 4], 0x0000);
    // mario.pushMaskedImage(0, 0, 16, 32, (uint16_t *)epd_bitmap_allArray[mario_idx % 3 + 4], (unsigned char *)epd_bitmap_alphaallArray[mario_idx % 3 + 4]);
    // mario.pushSprite(x, 0, TFT_BLACK);
    delay(100);
  }
  // delay(1000);
}