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

  tft.init();
  tft.setRotation(1);
  tft.setSwapBytes(true);

  background.createSprite(TFT_HEIGHT, TFT_WIDTH);
  background.setSwapBytes(tft.getSwapBytes());

  Serial.println("Initialized");
  mario.createSprite(16, 32);
}

void loop(void)
{
  for (int x = 0, mario_idx = 0; x < TFT_HEIGHT - 16; x++, mario_idx++)
  {
    background.fillSprite(TFT_OLIVE);
    mario.pushImage(0, 0, 16, 32, (uint16_t *)epd_bitmap_allArray[mario_idx % 3]);
    // The transparency masked is swapped 0x03ae -> 0xae03.
    // Not quite sure why. Probably swapping the bytes.
    // & is automatic because of the overflow in C++
    // (mario << 8 | mario >> 8) & (2**16 -1)
    mario.pushToSprite(&background, x, 0, 0xae03);

    background.pushSprite(0, 0);

    delay(100);
  }
}