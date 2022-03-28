/************* F R I E N D S H I P E N D E R **************
 * @file main.c
 * 智能电子墨水时钟 emw3
 * 使用前,请修改 ESP8266SdFat 库中 ENABLE_DEDICATED_SPI 的值为 0
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
  在开发之前请仔细参考 readme.txt
 */
#include <Arduino.h>
#include <SPI.h>
#include <FS.h>
#include <SD.h>
#include <ESP8266SDUpdater.h>
#include <GxEPD2_BW.h>
#include "emw3.h"
GxEPD2_BW<GxEPD2_213, GxEPD2_213::HEIGHT> 
epd(GxEPD2_213(/*CS*/ _DEFINA_EPD_CS_PIN, /*DC*/ _DEFINA_EPD_DC_PIN,
/*RST*/ _DEFINA_EPD_RST_PIN, /*BUSY*/ -1));
#include <Fonts/FreeMonoBold9pt7b.h>
#include <U8g2_for_Adafruit_GFX.h>

emw3_base emw3;
U8G2_FOR_ADAFRUIT_GFX cn;
bool sd_in =0;
void setup(){
  Serial.begin(115200);
  Serial.println("\nInput new lut data.");
  emw3.begin(NULL);
  epd.fillScreen(GxEPD_WHITE);
  emw3.blockedDisplay(0);
  delay(3000);
  if(!SD.begin(_DEFINA_SD_CS_PIN)){
    sd_in = 0;
    Serial.println("SD not available!");
    digitalWrite(D4,LOW);
    delay(500);
    digitalWrite(D4,HIGH);
  }
  else sd_in =1;
  epd.setFont(&FreeMonoBold9pt7b);
  pinMode(D4,OUTPUT);
  digitalWrite(D4,HIGH);
  delay(1000);
  digitalWrite(D4,LOW);
  delay(20);
  digitalWrite(D4,HIGH);
  uint8_t r=0;
  if(ESP.getResetReason() == "Deep-Sleep Wake"){
    r=1;
  }
  Serial.println("EPD init done.");
    epd.fillScreen(GxEPD_WHITE);
    epd.setCursor(10,20);
    if(r) epd.print("Hibernate Wake up!");
    else epd.print("Hello EMW3!");
    epd.setCursor(10,40);
    epd.print("EMW3 Hardware Test");
    epd.setCursor(10,60);
    if(sd_in) {
      epd.print("SD card init done.");
      epd.setCursor(10,80);
      epd.print("Press K1 to loadback.");
    }
    else epd.print("Insert SD card.");
    cn.setCursor(10,114);
    cn.print("你好,世界,你好,墨水时钟");
  emw3.performDisplay(0);
  for(int i=0;i<100;i++) {
    Serial.println();
    delay(50);
  }
  Serial.print("Press to load.");
    digitalWrite(D4,LOW);
    delay(20);
    digitalWrite(D4,HIGH);
  delay(980);
  if(emw3.getBtn(_DEFINA_BtnL)==0 && sd_in){
    epd.init(0,false);
      epd.setCursor(40,75);
      epd.print("Loading MENU.BIN");
    emw3.performDisplay(1);
    for(int i=0;i<20;i++) {
      Serial.println(digitalRead(_DEFINA_EPD_BUSY_PIN));
      delay(50);
    }
    if(SD.begin(_DEFINA_SD_CS_PIN)) updateFromSD();
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
  for(int i=0;i<4;i++){
    epd.setRotation(i);
    epd.setPartialWindow(0,0,epd.width(),epd.height());
      epd.fillScreen(GxEPD_WHITE);
      epd.fillRect(0,0,20,20,GxEPD_BLACK);
      epd.drawRect(epd.width()-20,0,20,20,GxEPD_BLACK);
      epd.drawRect(0,epd.height()-20,20,20,GxEPD_BLACK);
      epd.drawRect(epd.width()-20,epd.height()-20,20,20,GxEPD_BLACK);
      epd.setCursor(10,50);
      epd.print("Rotation ");
      epd.print(i);
    emw3.performDisplay(1);
    for(int i=0;i<20;i++) {
      Serial.println(digitalRead(_DEFINA_EPD_BUSY_PIN));
      delay(50);
    }
  }

    epd.fillScreen(GxEPD_WHITE);
    epd.fillTriangle(10,10,20,20,15,30,GxEPD_BLACK);
    epd.setCursor(10,40);
    epd.print("EPD busy trigger test");
    Serial.println("Full display....");
    
    emw3.performDisplay(0);
    //此for循环也可以用检测EPD_BUSY的电平实现定时显示
    for(int i=0;i<20;i++) {
      Serial.print(digitalRead(_DEFINA_EPD_BUSY_PIN));
      delay(50);
    }
    
    Serial.println("\nFull display done!!!!");
    for(int i = 0;i<epd.width();i++){
      epd.drawFastVLine(i,0,i%100+22,GxEPD_BLACK);
      emw3.performDisplay(1);
      delay(10);
    }
    emw3.blockedDisplay(1);
    while(digitalRead(_DEFINA_EPD_BUSY_PIN)==1) { //等待显示刷新完成...
      yield();
    }
    delay(1000);
  for(int times=0;times<4;times++){
  uint8_t btnlstate,btnmstate,btnrstate,sdstate=0;
  btnlstate=emw3.getBtn(_DEFINA_BtnL);
  btnmstate=emw3.getBtn(_DEFINA_BtnM);
  btnrstate=emw3.getBtn(_DEFINA_BtnR);
  //if(sd_in) {
    sdstate=SD.begin(_DEFINA_SD_CS_PIN);
    sdstate += SD.exists("/MENU.BIN");
  //}
  //else sdstate=255;
  Serial.print("btnmstate ");
  Serial.println(btnlstate);
  Serial.print("btnmstate ");
  Serial.println(btnmstate);
  Serial.print("btnrstate ");
  Serial.println(btnrstate);
  Serial.print("sdstate ");
  Serial.println(sdstate);
  epd.setRotation(1);
  if(times==0) {
    epd.fillScreen(GxEPD_WHITE);
    epd.fillTriangle(10,10,20,20,15,30,GxEPD_BLACK);
    Serial.println("Full display....");
    emw3.blockedDisplay(0);
    Serial.println("Full display done!!!!");
  }
    epd.fillScreen((times&1)?GxEPD_WHITE:GxEPD_BLACK);
    epd.setTextColor((times&1)?GxEPD_BLACK:GxEPD_WHITE);
    epd.setCursor(10,20);
    epd.print("Simple Test");
    epd.setCursor(10,40);
    epd.print("Button L(K1) State: ");
    epd.print(btnlstate);
    epd.setCursor(10,60);
    epd.print("Button M(K2) State: ");
    epd.print(btnmstate);
    epd.setCursor(10,80);
    epd.print("Button R(K3) State: ");
    epd.print(btnrstate);
    epd.setCursor(10,100);
    epd.print("SD File exists:");
    epd.print(sdstate);
    emw3.blockedDisplay(1);
    delay(1000);
    for(int i=0;i<20;i++) {
      Serial.println(digitalRead(_DEFINA_EPD_BUSY_PIN));
      delay(50);
    }
  Serial.println(times);
  }
  delay(500);
  epd.setFullWindow();
  File hello = SD.open("/hello.txt");
  String hellostr = "No data";
  if(hello) {
    hellostr = hello.readString();
    hello.close();
  }
    epd.fillScreen(GxEPD_WHITE);
    epd.setCursor(20,20);
    epd.print("hibernating");
    epd.setCursor(20,40);
    epd.print(hellostr);
    emw3.performDisplay(0);
    for(int i=0;i<100;i++) {
      Serial.println(digitalRead(_DEFINA_EPD_BUSY_PIN));
      delay(50);
    }
  epd.hibernate();
    digitalWrite(D4,LOW);
    delay(200);
    digitalWrite(D4,HIGH);
  ESP.deepSleep(10000000);
}
