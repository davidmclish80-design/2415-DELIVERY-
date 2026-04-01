#include <Wire.h>
#include "Adafruit_BMP280.h"
#include "Adafruit_Sensor.h"
#include "DHT.h"

// FASTLED LIBRARY
#include <FastLED.h>

#ifndef _WIFI_H
#include <WiFi.h>
#endif

#ifndef STDLIB_H
#include <stdlib.h>
#endif

#ifndef STDIO_H
#include <stdio.h>
#endif

#ifndef ARDUINO_H
#include <Arduino.h>
#endif

//////////////////////////////////////////////////////
// TFT LIBRARIES BELOW//
#include <SPI.h>
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <ArduinoJson.h>
//////////////////////////////////////////////////////

// MQTT CLIENT CONFIG
static const char* pubtopic      = "620171852";
static const char* subtopic[]    = {"620171852_sub", "/elet2415"};
static const char* mqtt_server   = "www.yanacreations.com";
static uint16_t mqtt_port        = 1883;

// WIFI CREDENTIALS
//const char* ssid = "ARRIS-F53D";
//const char* password = "70DFF79FF53D";
// const char* ssid = "gadfa’s iPhone";
// const char* password = "12345678";
const char* ssid = "MonaConnect";
const char* password = "";

// ---------- Prototypes needed by helper headers ----------
void callback(char* topic, byte* payload, unsigned int length);
void vUpdate(void *pvParameters);
void vButtonCheck(void *pvParameters);
unsigned long getTimeStamp(void);
bool publish(const char *topic, const char *payload);
void loopSerial(void);

// ---------- Task handles expected by helper headers ----------
TaskHandle_t xMQTT_Connect = NULL;
TaskHandle_t xNTPHandle = NULL;
TaskHandle_t xLOOPHandle = NULL;
TaskHandle_t xUpdateHandle = NULL;
TaskHandle_t xButtonCheckeHandle = NULL;

//############### IMPORT HEADER FILES ##################
#ifndef NTP_H
#include "NTP.h"
#endif

#ifndef MQTT_H
#include "mqtt.h"
#endif
////////////////////////////////////////////////////////





// WS2812 below
#define ledarray 13
#define NUM_LEDS 7
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB

CRGB leds[NUM_LEDS];

// BMP280 below
#define BMP_SCL 22
#define BMP_SDA 21

// DHT22 BELOW
#define DHTPIN 32
#define DHTTYPE DHT22

// C_MOISTURE_S BELOW
#define CMS_AOUT 34

int dryValue = 3183;
int wetValue = 2018;

//////////////////////////
// TFT PINS DEFINED BELOW//
#define TFT_DC    27
#define TFT_CS    5
#define TFT_RST   14
#define TFT_CLK   18
#define TFT_MOSI  23
#define TFT_MISO  19
/////////////////////////

Adafruit_BMP280 bmp;
DHT dht(DHTPIN, DHTTYPE);
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

// stores previous humidity reading
float previousHumidity = -1.0;

// ---------- Global sensor values used by loop(), vUpdate(), loopSerial() ----------
float temB = 0.0;
float PR = 0.0;
float Alt = 0.0;

float temD = 0.0;
float temDFaren = 0.0;
float humidity = 0.0;

float hif = 0.0;
float hic = 0.0;

int j = 0;
int percent = 0;

// ----------------------------
// Helper function to draw box
// ----------------------------
void drawReadingBox(int x, int y, int w, int h, String text) {
  tft.fillRect(x, y, w, h, ILI9341_BLACK);
  tft.drawRect(x, y, w, h, ILI9341_WHITE);
  tft.setCursor(x + 8, y + 9);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.print(text);
}

// ----------------------------
// Draw full TFT screen layout
// ----------------------------
void updateTFT(float temD, float humidity, float hic, float PR, float Alt, int percent) {
  tft.fillScreen(ILI9341_BLUE);

  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);

  String title = "SENSOR READINGS";
  int16_t x1, y1;
  uint16_t w, h;
  tft.getTextBounds(title, 0, 0, &x1, &y1, &w, &h);
  int titleX = (320 - w) / 2;
  tft.setCursor(titleX, 10);
  tft.print(title);

  String line1 = "TC:" + String(temD, 1) +
                 " H:" + String(humidity, 1) +
                 " HI:" + String(hic, 1);

  String line2 = "BMP P:" + String(PR, 0) +
                 " A:" + String(Alt, 1);

  String line3 = "CMS: Moisture%:" + String(percent);

  drawReadingBox(20, 45, 280, 40, line1);
  drawReadingBox(20, 105, 280, 40, line2);
  drawReadingBox(20, 165, 280, 40, line3);
}

// ----------------------------
// FASTLED helper functions
// ----------------------------
void setAllLEDs(CRGB color) {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = color;
  }
  FastLED.show();
}

void updateLEDFromHumidity(float humidity) {
  if (previousHumidity < 0) {
    previousHumidity = humidity;
    setAllLEDs(CRGB::Black);
    return;
  }

  float threshold = 0.2;

  if (humidity > previousHumidity + threshold) {
    setAllLEDs(CRGB::Red);
    Serial.println("Humidity increased -> LEDs RED");
  }
  else if (humidity < previousHumidity - threshold) {
    setAllLEDs(CRGB::Blue);
    Serial.println("Humidity decreased -> LEDs BLUE");
  }
  else {
    setAllLEDs(CRGB::Black);
    Serial.println("Humidity unchanged -> LEDs OFF");
  }

  previousHumidity = humidity;
}

void setup() {
  Serial.begin(115200);

  Wire.begin(BMP_SDA, BMP_SCL);
  dht.begin();

  while (!bmp.begin(0x76)) {
    Serial.print("Wrong Address OR circuit issues");
  }

  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(ILI9341_BLUE);

  FastLED.addLeds<LED_TYPE, ledarray, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(80);
  FastLED.clear();
  FastLED.show();

  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.setCursor(70, 110);
  tft.print("Starting sensors...");
  delay(1500);

  initialize();
}

void loop() {
  // BMP280
  temB = bmp.readTemperature();
  PR = bmp.readPressure();
  Alt = bmp.readAltitude(1015);

  // DHT22
  temD = dht.readTemperature();
  temDFaren = dht.readTemperature(true);
  humidity = dht.readHumidity();

  // Compute heat index
  hif = dht.computeHeatIndex(temDFaren, humidity);
  hic = dht.computeHeatIndex(temD, humidity, false);

  if (isnan(humidity) || isnan(temD) || isnan(temDFaren)) {
    Serial.println(F("Failed to read from DHT sensor!"));

    tft.fillScreen(ILI9341_BLUE);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(2);
    tft.setCursor(30, 120);
    tft.print("Failed to read DHT sensor");

    setAllLEDs(CRGB::Black);
    delay(2000);
    return;
  }

  // CMS
  j = analogRead(CMS_AOUT);
  percent = map(j, dryValue, wetValue, 0, 100);
  percent = constrain(percent, 0, 100);

  loopSerial();

  updateTFT(temD, humidity, hic, PR, Alt, percent);
  updateLEDFromHumidity(humidity);

  delay(2000);
}

//####################################################################
//#                          UTIL FUNCTIONS                          #
//####################################################################
void vButtonCheck(void * pvParameters) {
  configASSERT(((uint32_t) pvParameters) == 1);

  for (;;) {
    vTaskDelay(200 / portTICK_PERIOD_MS);
  }
}

void vUpdate(void * pvParameters) {
  configASSERT(((uint32_t) pvParameters) == 1);

  for (;;) {
    StaticJsonDocument<256> doc;
    char message[512] = {0};

    doc["id"] = "620171852";
    doc["timestamp"] = getTimeStamp();
    doc["temperature_dht_c"] = temD;
    doc["temperature_bmp_c"] = temB;
    doc["humidity_pct"] = humidity;
    doc["heat_index_c"] = hic;
    doc["pressure_pa"] = PR;
    doc["altitude_m"] = Alt;
    doc["moisture_pct"] = percent;

    serializeJson(doc, message);

    if (mqtt.connected()) {
      publish(pubtopic, message);
    }

    vTaskDelay(3000 / portTICK_PERIOD_MS);
  }
}

unsigned long getTimeStamp(void) {
  time_t now;
  time(&now);
  return (unsigned long) now;
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println("CALLBACK FIRED");
  Serial.printf("RX topic=%s len=%u\n", topic, length);
  Serial.printf("\nMessage received : ( topic: %s ) \n", topic);

  char *received = new char[length + 1] {0};

  for (unsigned int i = 0; i < length; i++) {
    received[i] = (char) payload[i];
  }

  Serial.printf("Payload : %s \n", received);

  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, received);

  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    delete[] received;
    return;
  }

  delete[] received;
}

bool publish(const char *topic, const char *payload) {
  bool res = false;
  try {
    res = mqtt.publish(topic, payload);
    if (!res) {
      res = false;
      throw false;
    }
  }
  catch (...) {
    Serial.printf("\nError (%d) >> Unable to publish message\n", res);
  }
  return res;
}

void loopSerial(void) {
  Serial.print(F("BMP280 reads temperature as ")); Serial.println(temB);
  Serial.print(F("Pressure in pascals is ")); Serial.println(PR);
  Serial.print(F("Altitude= ")); Serial.println(Alt);
  Serial.println();

  Serial.print(F("DHT reads temperature as ")); Serial.println(temD);
  Serial.print(F("DHT Fahrenheit temp= ")); Serial.println(temDFaren);
  Serial.print(F("DHT Humidity= ")); Serial.println(humidity);
  Serial.print(F("Heat Index: Cels | Fahren = ( "));
  Serial.print(hic);
  Serial.print(" | ");
  Serial.print(hif);
  Serial.println(" )");
  Serial.println();

  Serial.print(F("Raw value is: ")); Serial.println(j);
  Serial.print(F("Moisture% is: ")); Serial.println(percent);
  Serial.println();
}