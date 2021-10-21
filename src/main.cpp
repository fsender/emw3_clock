/************* F R I E N D S H I P E N D E R **************
 * @file main.c
 * 智能电子墨水时钟 emw3
 * ----------|\      /|
 *  \        | \    / |
 *   >       |  \  /  |
 *  /        |   \/   |
 * ----------|        |
 * |        |----------
 * |   /\   |        /
 * |  /  \  |       <
 * | /    \ |        \
 * |/      \|----------

 */
#define _SD_CS_PIN 0
#define _EPD_CS_PIN 15
#define _EPD_DC_PIN 5
#define _EPD_RST_PIN -1
#define _EPD_BUSY_PIN 4
#define BtnL 5
#define BtnM 12
#define BtnR 2

#include <Arduino.h>
#include <SPI.h>
#include <FS.h>
#include <SD.h>
#include <ESP8266SDUpdater.h>
#include <GxEPD2_BW.h>
GxEPD2_BW<GxEPD2_213, GxEPD2_213::HEIGHT> 
display(GxEPD2_213(/*CS*/ _EPD_CS_PIN, /*DC*/ _EPD_DC_PIN, /*RST*/ _EPD_RST_PIN, /*BUSY*/ _EPD_BUSY_PIN));
#include <Fonts/FreeMonoBold9pt7b.h>

bool sd_in =0;
uint8_t getBtn(uint8_t btn);
void setup(){
  Serial.begin(115200);
  Serial.println();
  pinMode(D4,OUTPUT);
  digitalWrite(D4,HIGH);
  delay(1000);
  digitalWrite(D4,LOW);
  delay(20);
  digitalWrite(D4,HIGH);
  if(!SD.begin(_SD_CS_PIN)){
    sd_in = 0;
    Serial.println("SD not available!");
    digitalWrite(D4,LOW);
    delay(500);
    digitalWrite(D4,HIGH);
  }
  else sd_in =1;
  display.init();
  uint8_t r=0;
  if(ESP.getResetReason() == "Deep-Sleep Wake"){
    r=1;
  }
  display.setRotation(1);
  display.setFullWindow();
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);
  Serial.println("EPD init done.");
  display.firstPage();
  do{
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(10,20);
    if(r) display.print("Hibernate Wake up!");
    else display.print("Hello EMW3!");
    display.setCursor(10,40);
    display.print("EMW3 Hardware Test");
    display.setCursor(10,60);
    if(sd_in) {
      display.print("SD card init done.");
      display.setCursor(10,80);
      display.print("Press K1 to loadback.");
    }
    else display.print("Insert SD card.");
  }while(display.nextPage());
  Serial.print("Press to load.");
    digitalWrite(D4,LOW);
    delay(20);
    digitalWrite(D4,HIGH);
  delay(980);
  if(getBtn(BtnL)==0 && sd_in){
    display.init(0,false);
    display.setPartialWindow(40,60,200,20);
    display.firstPage();
    do{
      display.setCursor(40,75);
      display.print("Loading MENU.BIN");
    }while(display.nextPage());
    updateFromSD();
    ESP.restart();
  }
  Serial.print(".");
    digitalWrite(D4,LOW);
    delay(20);
    digitalWrite(D4,HIGH);
  delay(480);
  Serial.println(".");
}

void loop(){
  display.init(0,false);
  for(int i=0;i<4;i++){
    display.setRotation(i);
    display.setPartialWindow(0,0,display.width(),display.height());
    display.firstPage();
    do{
      display.fillScreen(GxEPD_WHITE);
      display.fillRect(0,0,20,20,GxEPD_BLACK);
      display.drawRect(display.width()-20,0,20,20,GxEPD_BLACK);
      display.drawRect(0,display.height()-20,20,20,GxEPD_BLACK);
      display.drawRect(display.width()-20,display.height()-20,20,20,GxEPD_BLACK);
      display.setCursor(10,50);
      display.print("Rotation ");
      display.print(i);
    }while(display.nextPage());
    digitalWrite(D4,LOW);
    delay(20);
    digitalWrite(D4,HIGH);
    delay(480);
    digitalWrite(D4,LOW);
    delay(20);
    digitalWrite(D4,HIGH);
    delay(480);
  }

  for(int times=0;times<16;times++){
  uint8_t btnlstate,btnmstate,btnrstate,sdstate=0;
  btnlstate=getBtn(BtnL);
  btnmstate=getBtn(BtnM);
  btnrstate=getBtn(BtnR);
  if(sd_in) {
    SD.end();
    sdstate=SD.begin(_SD_CS_PIN);
    sdstate += SD.exists("/MENU.BIN");
  }
  else sdstate=255;
  Serial.print("btnmstate ");
  Serial.println(btnlstate);
  Serial.print("btnmstate ");
  Serial.println(btnmstate);
  Serial.print("btnrstate ");
  Serial.println(btnrstate);
  Serial.print("sdstate ");
  Serial.println(sdstate);

  display.init(0,false);
  display.setRotation(1);
    SD.end();
    sdstate=SD.begin(_SD_CS_PIN);
    sdstate+=SD.exists("/MENU.BIN");
    
  if(times==0) display.setFullWindow();
  else display.setPartialWindow(0,0,250,122);
  display.firstPage();
  do{
    display.fillScreen((times&1)?GxEPD_WHITE:GxEPD_BLACK);
    display.setTextColor((times&1)?GxEPD_BLACK:GxEPD_WHITE);
    display.setCursor(10,20);
    display.print("Simple Test");
    display.setCursor(10,40);
    display.print("Button L(K1) State: ");
    display.print(btnlstate);
    display.setCursor(10,60);
    display.print("Button M(K2) State: ");
    display.print(btnmstate);
    display.setCursor(10,80);
    display.print("Button R(K3) State: ");
    display.print(btnrstate);
    display.setCursor(10,100);
    display.print("SD File exists:");
    display.print(sdstate);
  }
  while(display.nextPage());
  Serial.println(times);
  delay(1000);
  }
  delay(500);
  display.setFullWindow();
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(20,20);
    display.print("hibernating");
  }
  while (display.nextPage());
  display.hibernate();
  ESP.deepSleep(10000000);
}

uint8_t getBtn(uint8_t btn){
  pinMode(btn,INPUT_PULLUP);
  return digitalRead(btn);
}