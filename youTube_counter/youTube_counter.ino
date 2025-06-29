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
    display.setFont(&FreeSansBold9pt7b);
    display.setTextColor(GxEPD_BLACK);

    display.setCursor(20, 50);
    display.print("Subscribers:");
    display.setCursor(180, 50);
    display.print(stats.subscribers);

    display.setCursor(20, 90);
    display.print("Views:");
    display.setCursor(180, 90);
    display.print(stats.views);

  } while (display.nextPage());
}

void setup() {
  Serial.begin(115200);
  connectWiFi();
  YouTubeStats stats = fetchYouTubeStats();
  showStatsOnDisplay(stats);

  // Sleep for 15 minutes (optional)
  // esp_sleep_enable_timer_wakeup(15 * 60 * 1000000ULL);
  // esp_deep_sleep_start();

  
}

void loop() {
    if (millis() - lastUpdate > 1 * 60 * 1000UL) { // 15 minutes
    YouTubeStats stats = fetchYouTubeStats();
    showStatsOnDisplay(stats);
    lastUpdate = millis();
  }
}

