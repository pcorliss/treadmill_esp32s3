#include <TFT_eSPI.h> // Hardware-specific library
#include <SPI.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <NimBLEDevice.h>
#include <Free_Fonts.h>
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

// TFT_HEIGHT = 240
// TFT_WIDTH = 135

const int textOverlayWidth = 120;
const int textOverlayHeight = 85;
const int textOverlayX = 110;
const int textOverlayY = 25;

const int mario_x = 35;
const int mario_y = 30;

const uint8_t *INIT_SEQUENCE[4] = {
    (const uint8_t *)"\x02\x00\x00\x00\x00",
    (const uint8_t *)"\xC2\x00\x00\x00\x00",
    (const uint8_t *)"\xE9\xFF\x00\x00\x00",
    (const uint8_t *)"\xE4\x00\xF4\x00\x00"};
const int INIT_SEQUENCE_LEN = 4;

const uint8_t *startCmd = (const uint8_t *)"\xE1\x00\x00\x00\x00";
const uint8_t *stopCmd = (const uint8_t *)"\xE0\x00\x00\x00\x00";
const uint8_t *speedCmd = (const uint8_t *)"\xD0\x00\x00\x00\x00";

const uint8_t *distanceQuery = (const uint8_t *)"\xA1\x85\x00\x00\x00";
const uint8_t *timeQuery = (const uint8_t *)"\xA1\x89\x00\x00\x00";
const uint8_t *speedQuery = (const uint8_t *)"\xA1\x82\x00\x00\x00";

const uint8_t *queries[3] = {speedQuery, distanceQuery, timeQuery};

#include "NimBLEDevice.h"

NimBLEClient *pClient;
NimBLERemoteService *pSvc;
NimBLERemoteCharacteristic *pChr;
const NimBLEAddress treadmillAddress("00:0c:bf:3e:df:f9");
const NimBLEUUID subServiceUUID("fff0");
const NimBLEUUID characteristicUuid("0000fff1-0000-1000-8000-00805f9b34fb");

#define SPEED 1
#define DISTANCE 2
#define TIME 3
int lastCommand = 0;
uint8_t duration[3] = {0, 0, 0};
float distance = 0.0;
float speed = 0.0;

void display_freeram()
{
  Serial.print(F("- SRAM: "));
  Serial.println(ESP.getHeapSize());
  Serial.print(F("- SRAM left: "));
  Serial.println(ESP.getFreeHeap());
  Serial.print(F("- PSRAM: "));
  Serial.println(ESP.getPsramSize());
  Serial.print(F("- PSRAM left: "));
  Serial.println(ESP.getFreePsram());
}

void setup(void)
{
  Serial.begin(9600);
  delay(2000); // Let serial console settle
  Serial.println(F("Hello World"));
  display_freeram();

  // turn on backlite
  pinMode(TFT_BACKLITE, OUTPUT);
  digitalWrite(TFT_BACKLITE, HIGH);

  // turn on the TFT / I2C power supply
  pinMode(TFT_I2C_POWER, OUTPUT);
  digitalWrite(TFT_I2C_POWER, HIGH);
  delay(10);

  if (scale > 1)
  {
    Serial.println(F("Scaling Sprite Array"));
    scaleSpriteArray(epd_bitmap_mario, epd_bitmap_mario_LEN, 16, 32, scale);
    Serial.println(F("Mario Scaled"));
    display_freeram();
    scaleSpriteArray(epd_bitmap_cape, epd_bitmap_cape_LEN, 16, 16, scale);
    Serial.println(F("Cape Scaled"));
    display_freeram();
    // 512x45px Background Image
    // Too Large to allocate all to memory
    backgroundBuffer = (uint16_t *)malloc(TFT_HEIGHT * TFT_WIDTH * sizeof(uint16_t));
    Serial.println(F("Done Scaling Sprite Array"));
    display_freeram();
  }

  tft.init();
  tft.setRotation(1);
  tft.setSwapBytes(true);
  Serial.println(F("TFT Initialized"));
  display_freeram();

  background.createSprite(TFT_HEIGHT, TFT_WIDTH);
  background.setSwapBytes(tft.getSwapBytes());
  Serial.println(F("Background Initialized"));
  display_freeram();

  mario.createSprite(16 * scale, 32 * scale);
  cape.createSprite(16 * scale, 16 * scale);
  textOverlay.createSprite(textOverlayWidth, textOverlayHeight);
  textOverlayBuffer = (uint16_t *)malloc(textOverlayWidth * textOverlayHeight * sizeof(uint16_t));

  Serial.println(F("Sprites Initialized"));
  display_freeram();

  NimBLEDevice::init("");
  Serial.println(F("Initialized NimBLE"));
  NimBLEDevice::setPower(ESP_PWR_LVL_P9);
  Serial.println("Set power level");
  display_freeram();

  connectWifi();
  display_freeram();
}

void loop(void)
{
  Serial.println(F("Outer Loop"));
  display_freeram();

  if (!(pClient && pSvc && pChr && pClient->isConnected()))
  {
    // Need to figure out how to make this async
    connectToTreadmill();
  }

  for (int x = 0; x < 512; x++)
  {
    int mario_idx = (x / 2) % epd_bitmap_mario_LEN;
    int cape_idx = (x / 2) % epd_bitmap_cape_LEN;

    // background.fillSprite(TFT_OLIVE);
    // background.pushImage(0, 0, TFT_HEIGHT, TFT_WIDTH, (uint16_t *)epd_bitmap_bkg[0]);

    scaleChunkSprite(epd_bitmap_bkg[0], backgroundBuffer, 512, 45, TFT_HEIGHT, x, scale);
    background.pushImage(0, 0, TFT_HEIGHT, TFT_WIDTH, backgroundBuffer);
    // Serial.println(F("BKG Pushed"));
    // display_freeram();

    textOverlay.setSwapBytes(true);
    alphaBlendTextOverlay();
    textOverlay.pushImage(0, 0, textOverlayWidth, textOverlayHeight, textOverlayBuffer);
    // Serial.println(F("TXT Overlay Pushed"));
    // display_freeram();

    // textOverlay.setTextSize(1);
    textOverlay.setTextColor(TFT_WHITE);
    textOverlay.setCursor(0, 32);
    textOverlay.setTextWrap(true);
    textOverlay.setFreeFont(FF23);
    // textOverlay.println("Hello World ...");
    textOverlay.printf("%dh%02dm", duration[0], duration[1]);
    textOverlay.printf("%0.2fmi", distance);
    // Serial.println(F("TXT Rendered"));
    // display_freeram();

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
    // Serial.println(F("Mario Rendered "));
    // display_freeram();

    background.pushSprite(0, 0);
    // Serial.println(F("BKG Pushed"));
    // display_freeram();

    if (lastCommand == 0 && pSvc && pChr && pClient->isConnected())
    {
      Serial.println("Querying Treadmill");
      int cmd = x % 3;
      pChr->writeValue(queries[cmd], 5, false);
      lastCommand = cmd + 1;
    }

    delay(60);
  }
}

void notifyCB(NimBLERemoteCharacteristic *pRemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify)
{
  if (lastCommand == SPEED)
  {
    speed = decodeSpeed(pData);
    Serial.print("Speed: ");
    Serial.println(speed);
  }
  else if (lastCommand == DISTANCE)
  {
    distance = decodeDistance(pData);
    Serial.print("Distance: ");
    Serial.println(distance);
  }
  else if (lastCommand == TIME)
  {
    decodeTime(pData, duration);
    Serial.print("Time: ");
    Serial.print(duration[0]);
    Serial.print(":");
    Serial.print(duration[1]);
    Serial.print(":");
    Serial.println(duration[2]);
  }
  lastCommand = 0;
}

float decodeSpeed(uint8_t *data)
{
  // Assuming data[2] and data[3] are bytes representing the speed
  return data[2] + (data[3] / 100.0);
}

float decodeDistance(uint8_t *data)
{
  // Assuming data[2] and data[3] are bytes representing the distance
  return data[2] + (data[3] / 100.0);
}

void decodeTime(uint8_t *data, uint8_t *outputTime)
{
  // Assuming data[2], data[3], and data[4] are bytes representing hours, minutes, and seconds
  // Output is written to outputTime array passed as argument
  outputTime[0] = data[2]; // Hours
  outputTime[1] = data[3]; // Minutes
  outputTime[2] = data[4]; // Seconds
}

void connectToTreadmill()
{
  Serial.println("Attempting to connect");

  if (pClient)
  {
    Serial.println("Deleting existing client");
    pClient->end();
    NimBLEDevice::deleteClient(pClient);
    pClient = nullptr;
  }

  Serial.println("Creating new client");
  pClient = NimBLEDevice::createClient(treadmillAddress);
  pClient->setConnectTimeout(1);

  if (!pClient->connect())
  {
    Serial.println("Failed to connect to device");
    return;
  }

  Serial.println("Connected to device");
  pSvc = pClient->getService(subServiceUUID);
  if (!pSvc)
  {
    Serial.println("Failed to find service");
    return;
  }

  pChr = pSvc->getCharacteristic(characteristicUuid.toString());
  if (!pChr)
  {
    Serial.println("Failed to find characteristic");
    return;
  }

  Serial.println("Found characteristic");
  Serial.println("Sending Init Sequence");

  for (int i = 0; i < INIT_SEQUENCE_LEN; i++)
  {
    pChr->writeValue(INIT_SEQUENCE[i], 5, false);
  }
  Serial.println("Init Sequence Sent");

  if (!pChr->canNotify())
  {
    Serial.println("Characteristic doesn't support notifications");
    return;
  }

  if (!pChr->subscribe(false, notifyCB))
  {
    Serial.print("subscribe failed");
    return;
  }

  Serial.println("Connected and subscribed");
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
