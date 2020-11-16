//  Arduino knihovny:
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

// Define hardware typu, počtu matic, output pinu:
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4
#define CS_PIN 4

#define DATA_PIN 13
#define CLK_PIN 14

//konfigurace internetu
const char* ssid     = "";    
const char* password = "";

WiFiUDP ntpUDP;
// Create a new instance of the MD_Parola class with hardware SPI connection:
MD_Parola myDisplay = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

void setup() {
  // Inicializace
  myDisplay.begin();
  // Jas displeje (0-15)
  myDisplay.setIntensity(0);
  // Vymazání displeje
  myDisplay.displayClear();
  myDisplay.displayText("Testovaci text", PA_CENTER, 100, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT); //(pozice, rychlost, pauza, efekt IN, efekt OUT)
}
void loop() {
  if (myDisplay.displayAnimate()) {
    myDisplay.displayReset();
  }
}