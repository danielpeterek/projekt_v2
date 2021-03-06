 //  Arduino knihovny:
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncTCP.h>
#include <FS.h>

// Definice hardware typu, počtu matic, output pinu:
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW //kaskádový styl matice
#define MAX_DEVICES 4
#define CS_PIN 4
#define DATA_PIN 13
#define CLK_PIN 14


// ****** UTF8-Decoder: převedení řetězce UTF8 na extended ASCII *******
static byte c1;  // Vyrovnávací paměť posledního znaku
// Převedení jednoho znaku UTF8 na Extended ASCII
// Return "0" pokud bajt musí být ignorován
byte utf8ascii(byte ascii) {
    if ( ascii<128 ) {  // Standartní ASCII-set 0..0x7F zpracování
      c1=0;
      return( ascii );
    }

    // získaní předchozího vstupu
    byte last = c1;   // získání posledního znaku
    c1=ascii;         // paměť aktuálního znaku

    switch (last){     // převod závislý na prvním UTF8 znaku
      case 0xC2: return  (ascii);  break;
      case 0xC3: return  (ascii | 0xC0);  break;
      case 0x82: if(ascii==0xAC) return(0x80);       // Euro-symbol
    }
    return  (0);                                     // jinak: vrátit nulu, pokud je třeba znak ignorovat
}

// převedení String objektu z UTF8 String na Extended ASCII
String utf8ascii(String s){      
    String r="";
    char c;
    for (int i=0; i<s.length(); i++){
        c = utf8ascii(s.charAt(i));
        if (c!=0) r+=c;
    }
    return r;
}

// v místě převodu UTF8-string na Extended ASCII (ASCII je kratší!)
void utf8ascii(char* s){      
    int k=0;
    char c;
    for (int i=0; i<strlen(s); i++){
          c = utf8ascii(s[i]);
          if (c!=0)
          s[k++]=c;
    }
    s[k]=0;
}


//konfigurace internetu
const char* ssid     = "****";    
const char* password = "****";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
//NTPClient timeClient(ntpUDP, "192.168.1.1", 3600, 60000);
MD_Parola DotMatrix = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);
AsyncWebServer server(80); //port 80

//parametry displeje
uint8_t scrollSpeed = 50;
textEffect_t scrollEffect = PA_SCROLL_LEFT;
textPosition_t scrollAlign = PA_LEFT;
uint16_t scrollPause = 3000; //v milisekundách

long currentMillis = 0;
long previousMillis = 0;
int interval = 1000;

String text = "";
String formattedDate;
String timeStamp, hour, minute, second;
String dateStamp, year, month, date;
char dateBuffer[] = "";
String monthArray[12] = {
  " Led ", " Uno ", " Bre ", " Dub ", " Kve ", " Cvn ",
  " Cvc ", " Srp ", " Zar ", " Rij ", " Lis ", " Pro "
};
byte mode = 0;

enum {TIME, DATE, TEXT};
boolean displayMode = TIME;

void showDate(){
  // výpis data
    year = formattedDate.substring(0, 4);
    month = formattedDate.substring(5, 7);
    month = monthArray[month.toInt() - 1];
    date = formattedDate.substring(8, 10);
    date = String(date.toInt());
    dateStamp =  date + "." + month + year;
    dateStamp.toCharArray(dateBuffer, dateStamp.length()+1);
}

void showTime(){
  // výpis času
    hour = formattedDate.substring(11, 13);
    minute = formattedDate.substring(14, 16);
    second = formattedDate.substring(17, 19);
}

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

  DotMatrix.begin(); // vyvolání matice
  DotMatrix.setIntensity(0); //jas matice

  if(!SPIFFS.begin()){
      Serial.println("An Error has occurred while mounting SPIFFS");
      return;
  }

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String());
  });

  server.on("/text", HTTP_POST, [](AsyncWebServerRequest *request) {    
    text = request->arg("text").c_str(); 
    text = utf8ascii(text);
    Serial.println(text);
    mode = 1;
    displayMode = TEXT;
    request->send_P(200, "text/json", "{\"result\":\"ok\"}");
  });

  server.on("/index.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/js/index.js", String());
  });

  server.on("/clock", HTTP_POST, [](AsyncWebServerRequest *request){
    mode = 0;
    displayMode = TIME;
    request->send(200, "text/json", "{\"result\":\"ok\"}");
  });

  server.begin();

  timeClient.begin(); //vyvolání NTP klientu
  timeClient.setTimeOffset(3600); // offset čas v sekundách, GMT + 1 = 3600

  DotMatrix.displayText("ESP", scrollAlign, 70, scrollPause, scrollEffect, scrollEffect);
  displayMode = DATE;
}

void loop()
{
  while (!timeClient.update()) {
    timeClient.forceUpdate();
  } //platný datum a čas

  if (displayMode == DATE && DotMatrix.displayAnimate()) {
    DotMatrix.displayReset();
    displayMode = TIME;
  }

  currentMillis = millis();
  if (currentMillis - previousMillis > interval &&
      displayMode == TIME) {
    previousMillis = millis();
    formattedDate = timeClient.getFormattedDate(); //převedení data a času a čitelný formát
    Serial.println(formattedDate); //výpis převedeného času

    showDate();
    showTime();

    if (hour.toInt() == 0) {
      hour = String(hour.toInt() + 12);
    }
    else if (hour.toInt() < 13) {
      hour = String(hour.toInt());
    }
   
    if (second.toInt() == 0) { //pokud se sekundy rovnají nule, vypíše se aktuální datum
      displayMode = DATE;
      DotMatrix.displayClear();
      DotMatrix.displayText(dateBuffer, scrollAlign, scrollSpeed, scrollPause, scrollEffect, scrollEffect); //výpis data
      return;
    }
    else if (second.toInt() % 2) { //pokud je sekunda lichá, vypíše dvojtečku
      timeStamp = hour + ":" + minute;
    }
    else { //pokud je sudá, nevypíše dvojtečku
      timeStamp = hour + " " + minute;
    }
    
    DotMatrix.setTextAlignment(PA_CENTER); //zarovnání času uprostřed

    if(mode == 0){
      displayMode = TIME;
      DotMatrix.print(timeStamp); //výpis času
    }
    else if(mode == 1){
      displayMode = TEXT;
      DotMatrix.displayText(text.c_str(), scrollAlign, scrollSpeed, scrollPause, scrollEffect, scrollEffect); //výpis textu
    }
  }
}
