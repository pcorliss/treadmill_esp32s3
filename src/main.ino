#include <TFT_eSPI.h> // Hardware-specific library
#include <SPI.h>
#include <WiFi.h>
#include <mario.h>

TFT_eSPI tft = TFT_eSPI(); // Invoke custom library
TFT_eSprite mario = TFT_eSprite(&tft);
TFT_eSprite cape = TFT_eSprite(&tft);
TFT_eSprite background = TFT_eSprite(&tft);
const int scale = 3;

const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;

const char *host = "www.example.org"; // This should not be changed
const int httpPort = 80;              // This should not be changed

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

  connectWifi();
}

void loop(void)
{
  for (int x = 0, mario_idx = 0, cape_idx = 0; x < TFT_HEIGHT - (16 * scale); x += scale, mario_idx++, cape_idx++)
  {
    background.fillSprite(TFT_OLIVE);
    mario_idx %= epd_bitmap_mario_LEN;
    cape_idx %= epd_bitmap_cape_LEN;
    mario.pushImage(0, 0, 16 * scale, 32 * scale, (uint16_t *)epd_bitmap_mario[mario_idx]);
    cape.pushImage(0, 0, 16 * scale, 16 * scale, (uint16_t *)epd_bitmap_cape[cape_idx]);
    // The transparency masked is swapped 0x03ae -> 0xae03.
    // Not quite sure why. Probably swapping the bytes.
    // & is automatic because of the overflow in C++
    // (mario << 8 | mario >> 8) & (2**16 -1)
    // Cape offset by 10x10
    cape.pushToSprite(&background, x - (10 * scale), 0 + (10 * scale), 0xae03);
    mario.pushToSprite(&background, x, 0, 0xae03);

    background.pushSprite(0, 0);

    delay(125);
  }
  httpGet();
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

void httpGet()
{
  WiFiClient client;
  String footer = String(" HTTP/1.1\r\n") + "Host: " + String(host) + "\r\n" + "Connection: close\r\n\r\n";

  // WRITE --------------------------------------------------------------------------------------------
  if (!client.connect(host, httpPort))
  {
    Serial.println("Failed to connect!");
    return;
  }

  client.print("GET /" + footer);
  readResponse(&client);
}
