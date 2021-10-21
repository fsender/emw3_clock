#define ENABLE_GxEPD2_GFX 0

#include <GxEPD2_BW.h>
//#include <GxEPD2_3C.h>
//#include <GxEPD2_7C.h>

#include <Fonts/FreeMonoBold9pt7b.h>
//#include "GxEPD2_display_selection.h"
//#include "GxEPD2_display_selection_added.h"
//#include "GxEPD2_display_selection_new_style.h"
GxEPD2_BW<GxEPD2_213, GxEPD2_213::HEIGHT> display(GxEPD2_213(/*CS=D8*/D8, /*DC=D3*/D1, /*RST=D4*/-1, /*BUSY=D2*/D2)); // GDEW0213I5F

void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("setup");
  delay(100);
  display.init();
  display.setRotation(1);
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);
  int16_t tbx, tby; uint16_t tbw, tbh;
  display.getTextBounds("Hello World!", 0, 0, &tbx, &tby, &tbw, &tbh);
  // center the bounding box by transposition of the origin:
   uint16_t x = ((display.width() - tbw) / 2) - tbx;
   uint16_t y = ((display.height() - tbh) / 2) - tby;
  /*display.setFullWindow();
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(x,y);
    display.print("Hello World!");
  }
  while (display.nextPage());
  delay(3000);*/
  display.setPartialWindow(0,0,250,122);
}
int i=0;
void loop()
{
  display.firstPage();
  do
  {
    display.drawFastVLine(i%250,0,122,GxEPD_BLACK);
  }
  while (display.nextPage());
  if(i==249){
    display.setFullWindow();
    display.firstPage();
    do
    {
      display.drawFastVLine(i%250,0,122,GxEPD_BLACK);
    }
    while (display.nextPage());
    pinMode(D6,INPUT_PULLUP);
    for(int i=0;i<100;i++){
      Serial.println(digitalRead(D6));
      delay(40);
    }
    display.setPartialWindow(0,0,250,122);
  }
  else {
    Serial.print("filling i:");
    Serial.print(i);
    Serial.print("PinRead: ");
    pinMode(D6,INPUT_PULLUP);
    Serial.println(digitalRead(D6));
  }
  i++;
}
