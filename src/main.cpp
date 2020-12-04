 //  Arduino knihovny:
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <WiFi.h>
#include <WiFiClient.h>

// Definice hardware typu, počtu matic, output pinu:
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
// #define HARDWARE_TYPE MD_MAX72XX::ICSTATION_HW
#define MAX_DEVICES 4
#define CS_PIN 4
#define DATA_PIN 13
#define CLK_PIN 14

//konfigurace internetu
const char* ssid     = "Skynet";    
const char* password = "PIDD57361";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
MD_Parola DotMatrix = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

uint8_t scrollSpeed = 50;
textEffect_t scrollEffect = PA_SCROLL_LEFT;
textPosition_t scrollAlign = PA_LEFT;
uint16_t scrollPause = 3000; // milisekundy

long currentMillis = 0;
long previousMillis = 0;
int interval = 1000;

String formattedDate;
String timeStamp, hour, minute, second;
String dateStamp, year, month, date;
char dateBuffer[] = "";
String monthArray[12] = {
  " Led ", " Uno ", " Bre ", " Dub ", " Kve ", " Cvn ",
  " Cvc ", " Srp ", " Zar ", " Rij ", " Lis ", " Pro "
};
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
  Serial.println("WiFi pripojena.");
  Serial.println("IP adresa: ");
  Serial.println(WiFi.localIP());

  DotMatrix.begin();
  DotMatrix.setIntensity(0);

  timeClient.begin();
  timeClient.setTimeOffset(3600); // offset čas v sekundách, GMT = 3600

  DotMatrix.displayText("ESP hodiny", scrollAlign, scrollSpeed, scrollPause, scrollEffect, scrollEffect);
  displayMode = DATE;
}

void loop()
{
  while (!timeClient.update()) {
    timeClient.forceUpdate();
  }

  if (displayMode == DATE && DotMatrix.displayAnimate()) {
    DotMatrix.displayReset();
    displayMode = TIME;
  }

  currentMillis = millis();
  if (currentMillis - previousMillis > interval &&
      displayMode == TIME) {
    previousMillis = millis();
    formattedDate = timeClient.getFormattedDate();
    Serial.println(formattedDate);

    // výpis datumu
    year = formattedDate.substring(0, 4);
    month = formattedDate.substring(5, 7);
    month = monthArray[month.toInt() - 1];
    date = formattedDate.substring(8, 10);
    date = String(date.toInt());
    dateStamp = year + ", " + date + month;
    dateStamp.toCharArray(dateBuffer, dateStamp.length()+1);
    
    // výpis času
    hour = formattedDate.substring(11, 13);
    minute = formattedDate.substring(14, 16);
    second = formattedDate.substring(17, 19);

    if (hour.toInt() == 0) {
      hour = String(hour.toInt() + 12);
    }
    else if (hour.toInt() < 13) {
      hour = String(hour.toInt());
    }
    else {
      hour = String(hour.toInt() - 12);
    }

    if (second.toInt() == 0) {
      displayMode = DATE;
      DotMatrix.displayClear();
      DotMatrix.displayText(dateBuffer, scrollAlign, scrollSpeed, scrollPause, scrollEffect, scrollEffect);
      return;
    }
    else if (second.toInt() % 2) {
      timeStamp = hour + ":" + minute;
    }
    else {
      timeStamp = hour + " " + minute;
    }
    
    DotMatrix.setTextAlignment(PA_CENTER);
    DotMatrix.print(timeStamp);
  }
}

