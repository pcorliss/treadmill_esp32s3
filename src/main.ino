#include <TFT_eSPI.h> // Hardware-specific library
#include <SPI.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <mario.h>

TFT_eSPI tft = TFT_eSPI(); // Invoke custom library
TFT_eSprite mario = TFT_eSprite(&tft);
TFT_eSprite cape = TFT_eSprite(&tft);
TFT_eSprite background = TFT_eSprite(&tft);
const int scale = 3;

const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;
const char *postUrl = URL;
const char *inputId = INPUTID;

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
    scaleSpriteArray(epd_bitmap_bkg, epd_bitmap_bkg_LEN, TFT_HEIGHT / 3, TFT_WIDTH / 3, scale);
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

  // connectWifi();
}

const int mario_x = 35;
const int mario_y = 30;
// Mario is 32px tall * 3 scale == 96px
// Screen is 135px tall
// Mario feet to bottom of screen is 135 - 96 - 30 == 9px

// 240 / 3 == 80
// 135 / 3 == 45

// Mario is 32px tall
// 1 px gap in sprite
// x3 scale
// 135 - (31 * 3 + 30) == 12px gap at bottom
// 12px / 3 == 4px

void loop(void)
{
  for (int x = 0; x < epd_bitmap_mario_LEN * epd_bitmap_cape_LEN; x++)
  {
    int mario_idx = x % epd_bitmap_mario_LEN;
    int cape_idx = x % epd_bitmap_cape_LEN;

    // background.fillSprite(TFT_OLIVE);
    background.pushImage(0, 0, TFT_HEIGHT, TFT_WIDTH, (uint16_t *)epd_bitmap_bkg[0]);
    mario.pushImage(0, 0, 16 * scale, 32 * scale, (uint16_t *)epd_bitmap_mario[mario_idx]);
    cape.pushImage(0, 0, 16 * scale, 16 * scale, (uint16_t *)epd_bitmap_cape[cape_idx]);
    // The transparency masked is swapped 0x03ae -> 0xae03.
    // Not quite sure why. Probably swapping the bytes.
    // & is automatic because of the overflow in C++
    // (mario << 8 | mario >> 8) & (2**16 -1)
    // Cape offset by 10x10
    cape.pushToSprite(&background, mario_x - (10 * scale), mario_y + (10 * scale), 0xae03);
    mario.pushToSprite(&background, mario_x, mario_y, 0xae03);

    background.pushSprite(0, 0);

    delay(125);
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
