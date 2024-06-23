#include <TFT_eSPI.h> // Hardware-specific library
#include <SPI.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <mario.h>

TFT_eSPI tft = TFT_eSPI(); // Invoke custom library
TFT_eSprite mario = TFT_eSprite(&tft);
TFT_eSprite cape = TFT_eSprite(&tft);
TFT_eSprite background = TFT_eSprite(&tft);
TFT_eSprite textOverlay = TFT_eSprite(&tft);
const int scale = 3;

const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;
const char *postUrl = URL;
const char *inputId = INPUTID;

uint16_t *backgroundBuffer;
uint16_t *textOverlayBuffer;

const int textOverlayWidth = 130;
const int textOverlayHeight = 105;
const int textOverlayX = 100;
const int textOverlayY = 15;

const int mario_x = 35;
const int mario_y = 30;

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

  if (scale > 1)
  {
    Serial.println("Scaling Sprite Array");
    scaleSpriteArray(epd_bitmap_mario, epd_bitmap_mario_LEN, 16, 32, scale);
    scaleSpriteArray(epd_bitmap_cape, epd_bitmap_cape_LEN, 16, 16, scale);
    // 512x45px Background Image
    // Too Large to allocate all to memory
    backgroundBuffer = (uint16_t *)malloc(TFT_HEIGHT * TFT_WIDTH * sizeof(uint16_t));
    Serial.println("Done Scaling Sprite Array");
  }

  tft.init();
  tft.setRotation(1);
  tft.setSwapBytes(true);

  background.createSprite(TFT_HEIGHT, TFT_WIDTH);
  background.setSwapBytes(tft.getSwapBytes());

  Serial.println("Initialized");
  mario.createSprite(16 * scale, 32 * scale);
  cape.createSprite(16 * scale, 16 * scale);
  textOverlay.createSprite(textOverlayWidth, textOverlayHeight);
  textOverlayBuffer = (uint16_t *)malloc(textOverlayWidth * textOverlayHeight * sizeof(uint16_t));

  // connectWifi();
}

void loop(void)
{
  for (int x = 0; x < 512; x++)
  {
    int mario_idx = (x / 2) % epd_bitmap_mario_LEN;
    int cape_idx = (x / 2) % epd_bitmap_cape_LEN;

    // background.fillSprite(TFT_OLIVE);
    // background.pushImage(0, 0, TFT_HEIGHT, TFT_WIDTH, (uint16_t *)epd_bitmap_bkg[0]);

    scaleChunkSprite(epd_bitmap_bkg[0], backgroundBuffer, 512, 45, TFT_HEIGHT, x, scale);
    background.pushImage(0, 0, TFT_HEIGHT, TFT_WIDTH, backgroundBuffer);

    textOverlay.setTextSize(1);
    textOverlay.setSwapBytes(true);
    alphaBlendTextOverlay();
    textOverlay.pushImage(0, 0, textOverlayWidth, textOverlayHeight, textOverlayBuffer);
    // textOverlay.fillSprite(TFT_DARKGREY);
    textOverlay.setTextColor(TFT_WHITE);
    // textOverlay.setTextColor(TFT_BLACK, TFT_WHITE);

    textOverlay.drawString(" !\"#$%&'()*+,-./0123456", 0, 0, 2);
    textOverlay.drawString("789:;<=>?@ABCDEFGHIJKL", 0, 16, 2);
    textOverlay.drawString("MNOPQRSTUVWXYZ[\\]^_`", 0, 32, 2);
    textOverlay.drawString("abcdefghijklmnopqrstuvw", 0, 48, 2);

    mario.pushImage(0, 0, 16 * scale, 32 * scale, epd_bitmap_mario[mario_idx]);
    cape.pushImage(0, 0, 16 * scale, 16 * scale, epd_bitmap_cape[cape_idx]);
    // The transparency masked is swapped 0x03ae -> 0xae03.
    // Not quite sure why. Probably swapping the bytes.
    // & is automatic because of the overflow in C++
    // (mario << 8 | mario >> 8) & (2**16 -1)
    // Cape offset by 10x10
    cape.pushToSprite(&background, mario_x - (10 * scale), mario_y + (10 * scale), 0xae03);
    mario.pushToSprite(&background, mario_x, mario_y, 0xae03);
    textOverlay.pushToSprite(&background, textOverlayX, textOverlayY);

    background.pushSprite(0, 0);

    delay(60);
  }
}

uint16_t *scaleSprite(uint16_t *img, int width, int height, int scale)
{
  // allocate memory for the new image
  int new_width = width * scale;
  int new_height = height * scale;
  uint16_t *new_img = (uint16_t *)malloc(new_width * new_height * sizeof(uint16_t));
  for (int i = 0; i < width; i++)
  {
    for (int j = 0; j < height; j++)
    {
      for (int k = 0; k < scale; k++)
      {
        for (int l = 0; l < scale; l++)
        {
          new_img[i * scale + k + (j * scale + l) * new_width] = img[i + j * width];
        }
      }
    }
  }

  return new_img;
}

void alphaBlendTextOverlay()
{
  for (int i = 0; i < textOverlayWidth; i++)
  {
    for (int j = 0; j < textOverlayHeight; j++)
    {
      int x = i + textOverlayX;
      int y = j + textOverlayY;

      int idx = i + j * textOverlayWidth;
      int background_idx = x + y * TFT_HEIGHT;
      textOverlayBuffer[idx] = tft.alphaBlend(0x80, backgroundBuffer[background_idx], TFT_BLACK);
    }
  }
}

void scaleChunkSprite(uint16_t *img, uint16_t *buffer, int imgWidth, int imgHeight, int chunkWidth, int offset, int scale)
{
  for (int i = 0; i < (chunkWidth / scale); i++)
  {
    for (int j = 0; j < imgHeight; j++)
    {
      for (int k = 0; k < scale; k++)
      {
        for (int l = 0; l < scale; l++)
        {
          int x = (i + offset) % imgWidth; // Needs to range from 0-imgWidth
          int idx_x = i * scale + k;
          int idx_y = j * scale + l;
          buffer[idx_x + idx_y * chunkWidth] = img[x + j * imgWidth];
        }
      }
    }
  }
}

void scaleSpriteArray(uint16_t **array, int length, int width, int height, int scale)
{
  for (int i = 0; i < length; i++)
  {
    array[i] = scaleSprite(array[i], width, height, scale);
  }
}

void connectWifi()
{
  Serial.println();
  Serial.println("******************************************************");
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void readResponse(WiFiClient *client)
{
  unsigned long timeout = millis();
  while (client->available() == 0)
  {
    if (millis() - timeout > 5000)
    {
      Serial.println(">>> Client Timeout !");
      client->stop();
      return;
    }
  }

  // Read all the lines of the reply from server and print them to Serial
  while (client->available())
  {
    String line = client->readStringUntil('\r');
    Serial.print(line);
  }

  Serial.printf("\nClosing connection\n\n");
}

// curl -i -X POST -d'entry.1047162164=1.234' https://docs.google.com/forms/u/0/d/e/1FAIpQLSdclU7-nN2EGPs2t-XDsZuKpluklEsXBPmTDs6GfbGyu3MoBg/formResponse
// postTreadData(1.234);
void postTreadData(float distance)
{
  WiFiClient client;
  HTTPClient http;
  String url = postUrl;
  String data = String(inputId) + "=" + String(distance);

  http.begin(url);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  int httpResponseCode = http.POST(data); // Send the actual POST request

  if (httpResponseCode > 0)
  {
    // String response = http.getString(); // Get the response to the request
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode); // Print return code
    // Serial.println(response);           // Print request answer
  }
  else
  {
    Serial.print("Error on sending POST: ");
    Serial.println(httpResponseCode);

    http.end();
  }
}
