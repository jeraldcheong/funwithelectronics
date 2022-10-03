/*
  Project: Dot matrix clock with NTP server using ESP32
  Board: ESP32 Dev Module (Node32 Lite)
  
  Connections:
  ESP32 | Dot Matrix
    RAW - VCC
    GND - GND
     27 - DIN
     26 - CS
     25 - CLK
  
  External libraries:
  - MD_MAX72XX by majicDesigns Version 3.2.1 (Manager)
  - MD_Parola by majicDesigns Version 3.3.0 (Manager)
  - NTPClient by Fabrice Weinberg Version 3.1.0 (Zip)
    https://github.com/taranais/NTPClient/archive/master.zip

    Code based on:
    https://randomnerdtutorials.com/esp32-date-time-ntp-client-server-arduino/
    https://microdigisoft.com/current-date-and-time-with-esp32-using-ntp-server-client-and-arduino-ide/
 */

#include <WiFi.h>
#include "time.h"
const char* ssid = "YOURSSID"; // WiFi name
const char* password = "YOURPASSWORD"; // WiFi password

const char* ntpServer = "YOUR_NTP_SERVER"; // NTP server
const long gmtOffset_sec = 28800;
const int   daylightOffset_sec = 0;

#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
// #define HARDWARE_TYPE MD_MAX72XX::ICSTATION_HW
#define MAX_DEVICES 4
#define CLK_PIN   25 // 18 or 25
#define DATA_PIN  27 // 16 or 27
#define CS_PIN    26 // 17 or 26


MD_Parola DotMatrix = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

uint8_t scrollSpeed = 50; // default frame delay value
textEffect_t scrollEffect = PA_SCROLL_LEFT;
textPosition_t scrollAlign = PA_LEFT;
uint16_t scrollPause = 3000; // in milliseconds
long currentMillis = 0;
long previousMillis = 0;
int interval = 1000;
String timeStamp, hour, minute, second;
String dateStamp;
char dateBuffer[] = "";
enum {TIME, DATE};
boolean displayMode = TIME;

void setup()
{
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  DotMatrix.begin();
  DotMatrix.setIntensity(0);

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
  DotMatrix.displayText("NTP Clock by JC", scrollAlign, scrollSpeed, scrollPause, scrollEffect, scrollEffect);
  displayMode = DATE;
  printLocalTime();
}

void loop()
{
  // delay(1000);
  printLocalTime();
}

void printLocalTime() {

  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }

  /*
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  Serial.print("Day of week: ");
  Serial.println(&timeinfo, "%A");
  Serial.print("Month: ");
  Serial.println(&timeinfo, "%B");
  Serial.print("Day of Month: ");
  Serial.println(&timeinfo, "%d");
  Serial.print("Year: ");
  Serial.println(&timeinfo, "%Y");
  Serial.print("Hour: ");
  Serial.println(&timeinfo, "%H");
  Serial.print("Hour (12 hour format): ");
  Serial.println(&timeinfo, "%I");
  Serial.print("Minute: ");
  Serial.println(&timeinfo, "%M");
  Serial.print("Second: ");
  Serial.println(&timeinfo, "%S");

  Serial.println("Time variables");
  char timeHour[3];
  strftime(timeHour,3, "%H", &timeinfo);
  Serial.println(timeHour);
  char timeWeekDay[10];
  strftime(timeWeekDay,10, "%A", &timeinfo);
  Serial.println(timeWeekDay);
  Serial.println();
  */
  
  if (displayMode == DATE && DotMatrix.displayAnimate()) {
    DotMatrix.displayReset();
    displayMode = TIME;
  }

  currentMillis = millis();
  if (currentMillis - previousMillis > interval &&
      displayMode == TIME) {
    previousMillis = millis();

    char workBuffer[10];
    strftime(workBuffer,10, "%Y", &timeinfo);
    dateStamp = String(workBuffer);
    strftime(workBuffer,10, "%b", &timeinfo);
    dateStamp.concat(" ");
    dateStamp.concat(String(workBuffer));
    strftime(workBuffer,10, "%d", &timeinfo);
    dateStamp.concat(" ");
    dateStamp.concat(String(workBuffer));
    // strftime(workBuffer,10, "%T", &timeinfo);
    // dateStamp.concat(" ");
    // dateStamp.concat(String(workBuffer));
    dateStamp.toCharArray(dateBuffer, dateStamp.length()+1);

    strftime(workBuffer,10, "%H", &timeinfo);
    hour = String(workBuffer);
    strftime(workBuffer,10, "%M", &timeinfo);
    minute = String(workBuffer);
    strftime(workBuffer,10, "%S", &timeinfo);
    second = String(workBuffer);

    if (second.toInt() == 0) {
      displayMode = DATE;
      DotMatrix.displayClear();
      DotMatrix.displayText(dateBuffer, scrollAlign, scrollSpeed, scrollPause, scrollEffect, scrollEffect);
      return;
    }
    else if (second.toInt() % 2) {
      timeStamp = String(hour);
      timeStamp.concat(":");
      timeStamp.concat(String(minute));
    }
    else {
      timeStamp = String(hour);
      timeStamp.concat(" ");
      timeStamp.concat(String(minute));
    }
    
    DotMatrix.setTextAlignment(PA_CENTER);
    DotMatrix.print(timeStamp);
  }
}