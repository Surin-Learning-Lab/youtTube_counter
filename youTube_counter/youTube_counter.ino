#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <GxEPD2_3C.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include "secrets.h"

#define EPD_SS 5
#define EPD_DC 17
#define EPD_RST 16
#define EPD_BUSY 4

GxEPD2_3C<GxEPD2_290_C90c, GxEPD2_290_C90c::HEIGHT> display(GxEPD2_290_C90c(EPD_SS, EPD_DC, EPD_RST, EPD_BUSY));

unsigned long lastUpdate = 0;

struct YouTubeStats {
  String subscribers;
  String views;
};

void connectWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected");
}

YouTubeStats fetchYouTubeStats() {
  YouTubeStats stats = {"N/A", "N/A"};
  String url = "https://www.googleapis.com/youtube/v3/channels?part=statistics&id=" + String(YT_CHANNEL_ID) + "&key=" + String(YT_API_KEY);

  HTTPClient http;
  http.begin(url);
  int httpCode = http.GET();

  if (httpCode != 200) {
    http.end();
    return stats;
  }

  String payload = http.getString();
  http.end();

  DynamicJsonDocument doc(2048);
  DeserializationError error = deserializeJson(doc, payload);
  if (error) return stats;

  JsonObject statsObj = doc["items"][0]["statistics"];
  stats.subscribers = statsObj["subscriberCount"].as<String>();
  stats.views = statsObj["viewCount"].as<String>();

  return stats;
}

void showStatsOnDisplay(const YouTubeStats& stats) {
  display.init(115200);
  display.setRotation(3);
  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);

    // Left red area
    int screenWidth = display.width();   // 296
    int screenHeight = display.height(); // 128
    int redAreaWidth = screenWidth * 2 / 3;
    display.fillRect(0, 0, redAreaWidth, screenHeight, GxEPD_RED);

    // YouTube triangle
    int centerX = redAreaWidth / 2;
    int centerY = screenHeight / 2;
    int triSize = 30;
    display.fillTriangle(
      centerX - triSize / 2, centerY - triSize,
      centerX - triSize / 2, centerY + triSize,
      centerX + triSize, centerY,
      GxEPD_WHITE
    );

    // Font settings
    display.setFont(&FreeSansBold9pt7b);
    display.setTextColor(GxEPD_BLACK);

    // Right-side boxes
    int boxX = redAreaWidth + 5;
    int boxWidth = screenWidth - boxX - 5;
    int boxHeight = 50;

    // SUBS box
    int subsY = 10;
    display.drawRect(boxX, subsY, boxWidth, boxHeight, GxEPD_BLACK);
    display.setCursor(boxX + 10, subsY + 18);
    display.print("SUBS:");
    display.setCursor(boxX + 10, subsY + 40);
    display.print(stats.subscribers);

    // VIEWS box
    int viewsY = subsY + boxHeight + 10;
    display.drawRect(boxX, viewsY, boxWidth, boxHeight, GxEPD_BLACK);
    display.setCursor(boxX + 10, viewsY + 18);
    display.print("VIEWS:");
    display.setCursor(boxX + 10, viewsY + 40);
    display.print(stats.views);

  } while (display.nextPage());
}


void setup() {
  Serial.begin(115200);
  connectWiFi();
  YouTubeStats stats = fetchYouTubeStats();
  showStatsOnDisplay(stats);
  lastUpdate = millis();
}

void loop() {
  if (millis() - lastUpdate > 5 * 60 * 1000UL) { // every 5 minutes
    YouTubeStats stats = fetchYouTubeStats();
    showStatsOnDisplay(stats);
    lastUpdate = millis();
  }
}
