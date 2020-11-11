#include <SPI.h>
#include "LedMatrix.h"
 
#define NUMBER_OF_DEVICES 4
#define CS_PIN 2
LedMatrix ledMatrix = LedMatrix(NUMBER_OF_DEVICES, CS_PIN);
 
void setup()
{
Serial.begin(115200);
ledMatrix.init();
ledMatrix.setIntensity(4); // range is 0-15
ledMatrix.setText("Testovaci text");
}
 
void loop()
{
ledMatrix.clear();
ledMatrix.scrollTextLeft();
ledMatrix.drawText();
ledMatrix.commit();
delay(200);
}