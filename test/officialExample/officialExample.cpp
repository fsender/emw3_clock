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


#if !defined(__AVR) && !defined(_BOARD_GENERIC_STM32F103C_H_)

// comment out unused bitmaps to reduce code space used
#include "bitmaps/Bitmaps200x200.h" // 1.54" b/w
#include "bitmaps/Bitmaps104x212.h" // 2.13" b/w flexible GDEW0213I5F
#include "bitmaps/Bitmaps128x250.h" // 2.13" b/w
#include "bitmaps/Bitmaps128x296.h" // 2.9"  b/w
#include "bitmaps/Bitmaps176x264.h" // 2.7"  b/w
#include "bitmaps/Bitmaps400x300.h" // 4.2"  b/w
#include "bitmaps/Bitmaps640x384.h" // 7.5"  b/w
// 3-color
#include "bitmaps/Bitmaps3c200x200.h" // 1.54" b/w/r
#include "bitmaps/Bitmaps3c104x212.h" // 2.13" b/w/r
#include "bitmaps/Bitmaps3c128x296.h" // 2.9"  b/w/r
#include "bitmaps/Bitmaps3c176x264.h" // 2.7"  b/w/r
#include "bitmaps/Bitmaps3c400x300.h" // 4.2"  b/w/r
#if defined(ESP8266) || defined(ESP32)
#include "bitmaps/WS_Bitmaps800x600.h" // 6.0"  grey
#endif

#else

#endif

// note for partial update window and setPartialWindow() method:
// partial update window size and position is on byte boundary in physical x direction
// the size is increased in setPartialWindow() if x or w are not multiple of 8 for even rotation, y or h for odd rotation
// see also comment in GxEPD2_BW.h, GxEPD2_3C.h or GxEPD2_GFX.h for method setPartialWindow()

const char HelloWorld[] = "Hello World!";
const char HelloArduino[] = "Hello Arduino!";
const char HelloEpaper[] = "Hello E-Paper!";

void helloWorld()
{
  //Serial.println("helloWorld");
  epd.setRotation(1);
  epd.setFont(&FreeMonoBold9pt7b);
  epd.setTextColor(GxEPD_BLACK);
  int16_t tbx, tby; uint16_t tbw, tbh;
  epd.getTextBounds(HelloWorld, 0, 0, &tbx, &tby, &tbw, &tbh);
  // center bounding box by transposition of origin:
  uint16_t x = ((epd.width() - tbw) / 2) - tbx;
  uint16_t y = ((epd.height() - tbh) / 2) - tby;
  // full window mode is the initial mode, set it anyway
  epd.setFullWindow();
  epd.fillScreen(GxEPD_WHITE);
  epd.setCursor(x, y);
  epd.print(HelloWorld);
  emw3.blockedDisplay(false); // full update
  //Serial.println("helloWorld done");
}

void helloWorldForDummiesNotPaged()
{
  //Serial.println("helloWorld");
  const char text[] = "Hello World!";
  // most e-papers have width < height (portrait) as native orientation, especially the small ones
  // in GxEPD2 rotation 0 is used for native orientation (most TFT libraries use 0 fix for portrait orientation)
  // set rotation to 1 (rotate right 90 degrees) to have enough space on small displays (landscape)
  epd.setRotation(1);
  // select a suitable font in Adafruit_GFX
  epd.setFont(&FreeMonoBold9pt7b);
  // on e-papers black on white is more pleasant to read
  epd.setTextColor(GxEPD_BLACK);
  // Adafruit_GFX has a handy method getTextBounds() to determine the boundary box for a text for the actual font
  int16_t tbx, tby; uint16_t tbw, tbh; // boundary box window
  epd.getTextBounds(text, 0, 0, &tbx, &tby, &tbw, &tbh); // it works for origin 0, 0, fortunately (negative tby!)
  // center bounding box by transposition of origin:
  uint16_t x = ((epd.width() - tbw) / 2) - tbx;
  uint16_t y = ((epd.height() - tbh) / 2) - tby;
  // full window mode is the initial mode, set it anyway
  epd.setFullWindow();
  // here we use full buffered drawing
  // only part of the display is used if page_height is less than HEIGHT
  epd.fillScreen(GxEPD_WHITE); // set the background to white (fill the buffer with value for white)
  epd.setCursor(x, y); // set the postition to start printing text
  epd.print(text); // print some text
  // tell the graphics class to transfer the buffer content (page) to the controller buffer
  // and to refresh to the screen
  emw3.blockedDisplay(false); // full update
  //Serial.println("helloWorld done");
}

void helloFullScreenPartialMode()
{
  //Serial.println("helloFullScreenPartialMode");
  const char fullscreen[] = "full screen update";
  const char fpm[] = "fast partial mode";
  const char spm[] = "slow partial mode";
  const char npm[] = "no partial mode";
  // full window mode is the initial mode, set it anyway
  epd.setFullWindow();
  epd.setRotation(1);
  epd.setFont(&FreeMonoBold9pt7b);
  epd.setTextColor(GxEPD_BLACK);
  const char* updatemode;
  if (epd.epd2.hasFastPartialUpdate)
  {
    updatemode = fpm;
  }
  else if (epd.epd2.hasPartialUpdate)
  {
    updatemode = spm;
  }
  else
  {
    updatemode = npm;
  }
  // do this outside of the loop
  int16_t tbx, tby; uint16_t tbw, tbh;
  // center update text
  epd.getTextBounds(fullscreen, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t utx = ((epd.width() - tbw) / 2) - tbx;
  uint16_t uty = ((epd.height() / 4) - tbh / 2) - tby;
  // center update mode
  epd.getTextBounds(updatemode, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t umx = ((epd.width() - tbw) / 2) - tbx;
  uint16_t umy = ((epd.height() * 3 / 4) - tbh / 2) - tby;
  // center HelloWorld
  epd.getTextBounds(HelloWorld, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t hwx = ((epd.width() - tbw) / 2) - tbx;
  uint16_t hwy = ((epd.height() - tbh) / 2) - tby;
  epd.fillScreen(GxEPD_WHITE);
  epd.setCursor(hwx, hwy);
  epd.print(HelloWorld);
  epd.setCursor(utx, uty);
  epd.print(fullscreen);
  epd.setCursor(umx, umy);
  epd.print(updatemode);
  emw3.blockedDisplay(true); // partial update
  //Serial.println("helloFullScreenPartialMode done");
}

void helloArduino()
{
  //Serial.println("helloArduino");
  epd.setRotation(1);
  epd.setFont(&FreeMonoBold9pt7b);
  epd.setTextColor(epd.epd2.hasColor ? GxEPD_RED : GxEPD_BLACK);
  int16_t tbx, tby; uint16_t tbw, tbh;
  // align with centered HelloWorld
  epd.getTextBounds(HelloWorld, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t x = ((epd.width() - tbw) / 2) - tbx;
  // height might be different
  epd.getTextBounds(HelloArduino, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t y = ((epd.height() / 4) - tbh / 2) - tby; // y is base line!
  // make the window big enough to cover (overwrite) descenders of previous text
  uint16_t wh = FreeMonoBold9pt7b.yAdvance;
  uint16_t wy = (epd.height() / 4) - wh / 2;
  // full window mode is the initial mode, set it anyway
  epd.setFullWindow();
  // DO NOT epd.fillScreen(GxEPD_WHITE); keep current content!
  epd.fillRect(0, wy, epd.width(), wh, GxEPD_WHITE); // clear rect instead
  epd.setCursor(x, y);
  epd.print(HelloArduino);
  emw3.blockedDisplay(true);
  delay(1000);
  //Serial.println("helloArduino done");
}

void helloEpaper()
{
  //Serial.println("helloEpaper");
  epd.setRotation(1);
  epd.setFont(&FreeMonoBold9pt7b);
  epd.setTextColor(epd.epd2.hasColor ? GxEPD_RED : GxEPD_BLACK);
  int16_t tbx, tby; uint16_t tbw, tbh;
  // align with centered HelloWorld
  epd.getTextBounds(HelloWorld, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t x = ((epd.width() - tbw) / 2) - tbx;
  // height might be different
  epd.getTextBounds(HelloEpaper, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t y = (epd.height() * 3 / 4) + tbh / 2; // y is base line!
  // make the window big enough to cover (overwrite) descenders of previous text
  uint16_t wh = FreeMonoBold9pt7b.yAdvance;
  uint16_t wy = (epd.height() * 3 / 4) - wh / 2;
  // full window mode is the initial mode, set it anyway
  epd.setFullWindow();
  // DO NOT epd.fillScreen(GxEPD_WHITE); keep current content!
  epd.fillRect(0, wy, epd.width(), wh, GxEPD_WHITE); // clear rect instead
  epd.setCursor(x, y);
  epd.print(HelloEpaper);
  emw3.blockedDisplay(true);
  //Serial.println("helloEpaper done");
}

#if defined(ESP8266) || defined(ESP32)
#include <StreamString.h>
#define PrintString StreamString
#else
class PrintString : public Print, public String
{
  public:
    size_t write(uint8_t data) override
    {
      return concat(char(data));
    };
};
#endif

void helloValue(double v, int digits)
{
  //Serial.println("helloValue");
  epd.setRotation(1);
  epd.setFont(&FreeMonoBold9pt7b);
  epd.setTextColor(epd.epd2.hasColor ? GxEPD_RED : GxEPD_BLACK);
  PrintString valueString;
  valueString.print(v, digits);
  int16_t tbx, tby; uint16_t tbw, tbh;
  epd.getTextBounds(valueString, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t x = ((epd.width() - tbw) / 2) - tbx;
  uint16_t y = (epd.height() * 3 / 4) + tbh / 2; // y is base line!
  // show what happens, if we use the bounding box for partial window
  uint16_t wx = (epd.width() - tbw) / 2;
  uint16_t wy = (epd.height() * 3 / 4) - tbh / 2;
  // full window mode is the initial mode, set it anyway
  epd.setFullWindow();
  // DO NOT epd.fillScreen(GxEPD_WHITE); keep current content!
  epd.fillRect(wx, wy, tbw, tbh, GxEPD_WHITE); // clear rect instead
  epd.setCursor(x, y);
  epd.print(valueString);
  emw3.blockedDisplay(true);
  delay(2000);
  // make the partial window big enough to cover the previous text
  uint16_t ww = tbw; // remember window width
  epd.getTextBounds(HelloEpaper, 0, 0, &tbx, &tby, &tbw, &tbh);
  // adjust, because HelloEpaper was aligned, not centered (could calculate this to be precise)
  ww = max(ww, uint16_t(tbw + 12)); // 12 seems ok
  wx = (epd.width() - tbw) / 2;
  // make the window big enough to cover (overwrite) descenders of previous text
  uint16_t wh = FreeMonoBold9pt7b.yAdvance;
  wy = (epd.height() * 3 / 4) - wh / 2;
  // DO NOT epd.fillScreen(GxEPD_WHITE); keep current content!
  epd.fillRect(wx, wy, ww, wh, GxEPD_WHITE); // clear rect instead
  epd.setCursor(x, y);
  epd.print(valueString);
  emw3.blockedDisplay(true);
  // alternately use the whole width for partial window
  //emw3.blockedDisplay(wx, wy, epd.width(), wh);
  //Serial.println("helloValue done");
}

void deepSleepTest()
{
  //Serial.println("deepSleepTest");
  const char hibernating[] = "hibernating ...";
  const char wokeup[] = "woke up";
  const char from[] = "from deep sleep";
  const char again[] = "again";
  epd.setRotation(1);
  epd.setFont(&FreeMonoBold9pt7b);
  epd.setTextColor(GxEPD_BLACK);
  int16_t tbx, tby; uint16_t tbw, tbh;
  // center text
  epd.getTextBounds(hibernating, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t x = ((epd.width() - tbw) / 2) - tbx;
  uint16_t y = ((epd.height() - tbh) / 2) - tby;
  epd.setFullWindow();
  epd.fillScreen(GxEPD_WHITE);
  epd.setCursor(x, y);
  epd.print(hibernating);
  emw3.blockedDisplay(false); // full update
  epd.hibernate();
  delay(5000);
  epd.getTextBounds(wokeup, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t wx = (epd.width() - tbw) / 2;
  uint16_t wy = (epd.height() / 3) + tbh / 2; // y is base line!
  epd.getTextBounds(from, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t fx = (epd.width() - tbw) / 2;
  uint16_t fy = (epd.height() * 2 / 3) + tbh / 2; // y is base line!
  epd.fillScreen(GxEPD_WHITE);
  epd.setCursor(wx, wy);
  epd.print(wokeup);
  epd.setCursor(fx, fy);
  epd.print(from);
  emw3.blockedDisplay(false); // full update
  delay(5000);
  epd.getTextBounds(hibernating, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t hx = (epd.width() - tbw) / 2;
  uint16_t hy = (epd.height() / 3) + tbh / 2; // y is base line!
  epd.getTextBounds(again, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t ax = (epd.width() - tbw) / 2;
  uint16_t ay = (epd.height() * 2 / 3) + tbh / 2; // y is base line!
  epd.fillScreen(GxEPD_WHITE);
  epd.setCursor(hx, hy);
  epd.print(hibernating);
  epd.setCursor(ax, ay);
  epd.print(again);
  emw3.blockedDisplay(false); // full update
  epd.hibernate();
  //Serial.println("deepSleepTest done");
}

void showBox(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool partial)
{
  //Serial.println("showBox");
  epd.setRotation(1);
  epd.setFullWindow();
  epd.fillScreen(GxEPD_WHITE);
  epd.fillRect(x, y, w, h, GxEPD_BLACK);
  if (partial)
  {
    emw3.blockedDisplay(true);
  }
  else
  {
    emw3.blockedDisplay(false); // full update
  }
  //Serial.println("showBox done");
}

void drawCornerTest()
{
  epd.setFullWindow();
  epd.setFont(&FreeMonoBold9pt7b);
  epd.setTextColor(GxEPD_BLACK);
  for (uint16_t r = 0; r <= 4; r++)
  {
    epd.setRotation(r);
    epd.fillScreen(GxEPD_WHITE);
    epd.fillRect(0, 0, 8, 8, GxEPD_BLACK);
    epd.fillRect(epd.width() - 18, 0, 16, 16, GxEPD_BLACK);
    epd.fillRect(epd.width() - 25, epd.height() - 25, 24, 24, GxEPD_BLACK);
    epd.fillRect(0, epd.height() - 33, 32, 32, GxEPD_BLACK);
    epd.setCursor(epd.width() / 2, epd.height() / 2);
    epd.print(epd.getRotation());
    emw3.blockedDisplay(false); // full update
    delay(2000);
  }
}

void drawFont(const char name[], const GFXfont* f)
{
  //epd.setRotation(0);
  epd.fillScreen(GxEPD_WHITE);
  epd.setTextColor(GxEPD_BLACK);
  epd.setFont(f);
  epd.setCursor(0, 0);
  epd.println();
  epd.println(name);
  epd.println(" !\"#$%&'()*+,-./");
  epd.println("0123456789:;<=>?");
  epd.println("@ABCDEFGHIJKLMNO");
  epd.println("PQRSTUVWXYZ[\\]^_");
  if (epd.epd2.hasColor)
  {
    epd.setTextColor(GxEPD_RED);
  }
  epd.println("`abcdefghijklmno");
  epd.println("pqrstuvwxyz{|}~ ");
}

void showFont(const char name[], const GFXfont* f)
{
  epd.setFullWindow();
  epd.setRotation(0);
  epd.setTextColor(GxEPD_BLACK);
  drawFont(name, f);
  emw3.blockedDisplay(false); // full update
}

// note for partial update window and setPartialWindow() method:
// partial update window size and position is on byte boundary in physical x direction
// the size is increased in setPartialWindow() if x or w are not multiple of 8 for even rotation, y or h for odd rotation
// see also comment in GxEPD2_BW.h, GxEPD2_3C.h or GxEPD2_GFX.h for method setPartialWindow()
// showPartialUpdate() purposely uses values that are not multiples of 8 to test this

void showPartialUpdate()
{
  // some useful background
  helloWorld();
  // use asymmetric values for test
  uint16_t box_x = 10;
  uint16_t box_y = 15;
  uint16_t box_w = 70;
  uint16_t box_h = 20;
  uint16_t cursor_y = box_y + box_h - 6;
  float value = 13.95;
  uint16_t incr = epd.epd2.hasFastPartialUpdate ? 1 : 3;
  epd.setFont(&FreeMonoBold9pt7b);
  epd.setTextColor(GxEPD_BLACK);
  // show where the update box is
  for (uint16_t r = 0; r < 4; r++)
  {
    epd.setRotation(r);
    epd.fillRect(box_x, box_y, box_w, box_h, GxEPD_BLACK);
    //epd.fillScreen(GxEPD_BLACK);
    emw3.blockedDisplay(true);
    delay(2000);
    epd.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
    emw3.blockedDisplay(true);
    delay(1000);
  }
  //return;
  // show updates in the update box
  for (uint16_t r = 0; r < 4; r++)
  {
    epd.setRotation(r);
    for (uint16_t i = 1; i <= 10; i += incr)
    {
      epd.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
      epd.setCursor(box_x, cursor_y);
      epd.print(value * i, 2);
      emw3.blockedDisplay(true);
      delay(500);
    }
    delay(1000);
    epd.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
    emw3.blockedDisplay(true);
    delay(1000);
  }
}


#ifdef _GxBitmaps200x200_H_
void drawBitmaps200x200()
{
#if defined(__AVR)
  const unsigned char* bitmaps[] =
  {
    logo200x200, first200x200 //, second200x200, third200x200, fourth200x200, fifth200x200, sixth200x200, senventh200x200, eighth200x200
  };
#elif defined(_BOARD_GENERIC_STM32F103C_H_)
  const unsigned char* bitmaps[] =
  {
    logo200x200, first200x200, second200x200, third200x200, fourth200x200, fifth200x200 //, sixth200x200, senventh200x200, eighth200x200
  };
#else
  const unsigned char* bitmaps[] =
  {
    logo200x200, first200x200, second200x200, third200x200, fourth200x200, fifth200x200, sixth200x200, senventh200x200, eighth200x200
  };
#endif
  if ((epd.epd2.panel == GxEPD2::GDEP015OC1) || (epd.epd2.panel == GxEPD2::GDEH0154D67))
  {
    bool m = epd.mirror(true);
    for (uint16_t i = 0; i < sizeof(bitmaps) / sizeof(char*); i++)
    {
      epd.fillScreen(GxEPD_WHITE);
      epd.drawInvertedBitmap(0, 0, bitmaps[i], epd.epd2.WIDTH, epd.epd2.HEIGHT, GxEPD_BLACK);
      emw3.blockedDisplay();
      delay(2000);
    }
    epd.mirror(m);
  }
  //else
  {
    bool mirror_y = (epd.epd2.panel != GxEPD2::GDE0213B1);
    epd.clearScreen(); // use default for white
    int16_t x = (int16_t(epd.epd2.WIDTH) - 200) / 2;
    int16_t y = (int16_t(epd.epd2.HEIGHT) - 200) / 2;
    for (uint16_t i = 0; i < sizeof(bitmaps) / sizeof(char*); i++)
    {
      epd.drawImage(bitmaps[i], x, y, 200, 200, false, mirror_y, true);
      delay(2000);
    }
  }
  bool mirror_y = (epd.epd2.panel != GxEPD2::GDE0213B1);
  for (uint16_t i = 0; i < sizeof(bitmaps) / sizeof(char*); i++)
  {
    int16_t x = -60;
    int16_t y = -60;
    for (uint16_t j = 0; j < 10; j++)
    {
      epd.writeScreenBuffer(); // use default for white
      epd.writeImage(bitmaps[i], x, y, 200, 200, false, mirror_y, true);
      epd.refresh(true);
      if (epd.epd2.hasFastPartialUpdate)
      {
        // for differential update: set previous buffer equal to current buffer in controller
        epd.epd2.writeScreenBufferAgain(); // use default for white
        epd.epd2.writeImageAgain(bitmaps[i], x, y, 200, 200, false, mirror_y, true);
      }
      delay(2000);
      x += 40;
      y += 40;
      if ((x >= int16_t(epd.epd2.WIDTH)) || (y >= int16_t(epd.epd2.HEIGHT))) break;
    }
    if (!epd.epd2.hasFastPartialUpdate) break; // comment out for full show
    break; // comment out for full show
  }
  epd.writeScreenBuffer(); // use default for white
  epd.writeImage(bitmaps[0], int16_t(0), 0, 200, 200, false, mirror_y, true);
  epd.writeImage(bitmaps[0], int16_t(int16_t(epd.epd2.WIDTH) - 200), int16_t(epd.epd2.HEIGHT) - 200, 200, 200, false, mirror_y, true);
  epd.refresh(true);
  delay(2000);
}
#endif

#ifdef _GxBitmaps104x212_H_
void drawBitmaps104x212()
{
#if !defined(__AVR)
  const unsigned char* bitmaps[] =
  {
    WS_Bitmap104x212, Bitmap104x212_1, Bitmap104x212_2, Bitmap104x212_3
  };
#else
  const unsigned char* bitmaps[] =
  {
    WS_Bitmap104x212, Bitmap104x212_1, Bitmap104x212_2, Bitmap104x212_3
  };
#endif
  if (epd.epd2.panel == GxEPD2::GDEW0213I5F)
  {
    for (uint16_t i = 0; i < sizeof(bitmaps) / sizeof(char*); i++)
    {
      epd.fillScreen(GxEPD_WHITE);
      epd.drawBitmap(0, 0, bitmaps[i], epd.epd2.WIDTH, epd.epd2.HEIGHT, GxEPD_BLACK);
      emw3.blockedDisplay();
      delay(2000);
    }
  }
}
#endif

#ifdef _GxBitmaps128x250_H_
void drawBitmaps128x250()
{
#if !defined(__AVR)
  const unsigned char* bitmaps[] =
  {
    Bitmap128x250_1, logo128x250, first128x250, second128x250, third128x250
  };
#else
  const unsigned char* bitmaps[] =
  {
    Bitmap128x250_1, logo128x250, first128x250, second128x250, third128x250
  };
#endif
  if ((epd.epd2.panel == GxEPD2::GDE0213B1) || (epd.epd2.panel == GxEPD2::GDEH0213B72))
  {
    bool m = epd.mirror(true);
    for (uint16_t i = 0; i < sizeof(bitmaps) / sizeof(char*); i++)
    {
      epd.fillScreen(GxEPD_WHITE);
      epd.drawInvertedBitmap(0, 0, bitmaps[i], epd.epd2.WIDTH, epd.epd2.HEIGHT, GxEPD_BLACK);
      emw3.blockedDisplay();
      delay(2000);
    }
    epd.mirror(m);
  }
}
#endif

#ifdef _GxBitmaps128x296_H_
void drawBitmaps128x296()
{
#if !defined(__AVR)
  const unsigned char* bitmaps[] =
  {
    Bitmap128x296_1, logo128x296, first128x296, second128x296, third128x296
  };
#else
  const unsigned char* bitmaps[] =
  {
    Bitmap128x296_1, logo128x296 //, first128x296, second128x296, third128x296
  };
#endif
  if (epd.epd2.panel == GxEPD2::GDEH029A1)
  {
    bool m = epd.mirror(true);
    for (uint16_t i = 0; i < sizeof(bitmaps) / sizeof(char*); i++)
    {
      epd.fillScreen(GxEPD_WHITE);
      epd.drawInvertedBitmap(0, 0, bitmaps[i], epd.epd2.WIDTH, epd.epd2.HEIGHT, GxEPD_BLACK);
      emw3.blockedDisplay();
      delay(2000);
    }
    epd.mirror(m);
  }
}
#endif

#ifdef _GxBitmaps176x264_H_
void drawBitmaps176x264()
{
#if !defined(__AVR)
  const unsigned char* bitmaps[] =
  {
    Bitmap176x264_1, Bitmap176x264_2, Bitmap176x264_3, Bitmap176x264_4, Bitmap176x264_5
  };
#else
  const unsigned char* bitmaps[] =
  {
    Bitmap176x264_1, Bitmap176x264_2 //, Bitmap176x264_3, Bitmap176x264_4, Bitmap176x264_5
  };
#endif
  if (epd.epd2.panel == GxEPD2::GDEW027W3)
  {
    for (uint16_t i = 0; i < sizeof(bitmaps) / sizeof(char*); i++)
    {
      epd.fillScreen(GxEPD_WHITE);
      epd.drawInvertedBitmap(0, 0, bitmaps[i], epd.epd2.WIDTH, epd.epd2.HEIGHT, GxEPD_BLACK);
      emw3.blockedDisplay();
      delay(2000);
    }
  }
}
#endif

#ifdef _GxBitmaps400x300_H_
void drawBitmaps400x300()
{
#if !defined(__AVR)
  const unsigned char* bitmaps[] =
  {
    Bitmap400x300_1, Bitmap400x300_2
  };
#else
  const unsigned char* bitmaps[] = {}; // not enough code space
#endif
  if (epd.epd2.panel == GxEPD2::GDEW042T2)
  {
    for (uint16_t i = 0; i < sizeof(bitmaps) / sizeof(char*); i++)
    {
      epd.fillScreen(GxEPD_WHITE);
      epd.drawInvertedBitmap(0, 0, bitmaps[i], epd.epd2.WIDTH, epd.epd2.HEIGHT, GxEPD_BLACK);
      emw3.blockedDisplay();
      delay(2000);
    }
  }
}
#endif

#ifdef _GxBitmaps640x384_H_
void drawBitmaps640x384()
{
#if !defined(__AVR)
  const unsigned char* bitmaps[] =
  {
    Bitmap640x384_1, Bitmap640x384_2
  };
#else
  const unsigned char* bitmaps[] = {}; // not enough code space
#endif
  if ((epd.epd2.panel == GxEPD2::GDEW075T8) || (epd.epd2.panel == GxEPD2::GDEW075Z09))
  {
    for (uint16_t i = 0; i < sizeof(bitmaps) / sizeof(char*); i++)
    {
      epd.fillScreen(GxEPD_WHITE);
      epd.drawInvertedBitmap(0, 0, bitmaps[i], epd.epd2.WIDTH, epd.epd2.HEIGHT, GxEPD_BLACK);
      emw3.blockedDisplay();
      delay(2000);
    }
  }
}
#endif

#ifdef _WS_Bitmaps800x600_H_
void drawBitmaps800x600()
{
#if defined(ESP8266) || defined(ESP32)
  if (epd.epd2.panel == GxEPD2::ED060SCT)
  {
    //    Serial.print("sizeof(WS_zoo_800x600) is "); Serial.println(sizeof(WS_zoo_800x600));
    epd.drawNative(WS_zoo_800x600, 0, 0, 0, 800, 600, false, false, true);
    delay(2000);
    //    Serial.print("sizeof(WS_pic_1200x825) is "); Serial.println(sizeof(WS_pic_1200x825));
    //    epd.drawNative((const uint8_t*)WS_pic_1200x825, 0, 0, 0, 1200, 825, false, false, true);
    //    delay(2000);
    //    Serial.print("sizeof(WS_acaa_1024x731) is "); Serial.println(sizeof(WS_acaa_1024x731));
    //    epd.drawNative(WS_acaa_1024x731, 0, 0, 0, 1024, 731, false, false, true);
    //    delay(2000);
  }
#endif
}
#endif

struct bitmap_pair
{
  const unsigned char* black;
  const unsigned char* red;
};

#ifdef _GxBitmaps3c200x200_H_
void drawBitmaps3c200x200()
{
  bitmap_pair bitmap_pairs[] =
  {
    //{Bitmap3c200x200_black, Bitmap3c200x200_red},
    {WS_Bitmap3c200x200_black, WS_Bitmap3c200x200_red}
  };
  if (epd.epd2.panel == GxEPD2::GDEW0154Z04)
  {
    epd.fillScreen(GxEPD_WHITE);
    // Bitmap3c200x200_black has 2 bits per pixel
    // taken from Adafruit_GFX.cpp, modified
    int16_t byteWidth = (epd.epd2.WIDTH + 7) / 8; // Bitmap scanline pad = whole byte
    uint8_t byte = 0;
    for (int16_t j = 0; j < epd.epd2.HEIGHT; j++)
    {
      for (int16_t i = 0; i < epd.epd2.WIDTH; i++)
      {
        if (i & 3) byte <<= 2;
        else
        {
#if defined(__AVR) || defined(ESP8266) || defined(ESP32)
          byte = pgm_read_byte(&Bitmap3c200x200_black[j * byteWidth * 2 + i / 4]);
#else
          byte = Bitmap3c200x200_black[j * byteWidth * 2 + i / 4];
#endif
        }
        if (!(byte & 0x80))
        {
          epd.drawPixel(i, j, GxEPD_BLACK);
        }
      }
    }
    epd.drawInvertedBitmap(0, 0, Bitmap3c200x200_red, epd.epd2.WIDTH, epd.epd2.HEIGHT, GxEPD_RED);
    emw3.blockedDisplay();
    delay(2000);
    for (uint16_t i = 0; i < sizeof(bitmap_pairs) / sizeof(bitmap_pair); i++)
    {
      epd.fillScreen(GxEPD_WHITE);
      epd.drawInvertedBitmap(0, 0, bitmap_pairs[i].black, epd.epd2.WIDTH, epd.epd2.HEIGHT, GxEPD_BLACK);
      epd.drawInvertedBitmap(0, 0, bitmap_pairs[i].red, epd.epd2.WIDTH, epd.epd2.HEIGHT, GxEPD_RED);
      emw3.blockedDisplay();
      delay(2000);
    }
  }
  if (epd.epd2.hasColor)
  {
    epd.clearScreen(); // use default for white
    int16_t x = (int16_t(epd.epd2.WIDTH) - 200) / 2;
    int16_t y = (int16_t(epd.epd2.HEIGHT) - 200) / 2;
    for (uint16_t i = 0; i < sizeof(bitmap_pairs) / sizeof(bitmap_pair); i++)
    {
      epd.drawImage(bitmap_pairs[i].black, bitmap_pairs[i].red, x, y, 200, 200, false, false, true);
      delay(2000);
    }
    for (uint16_t i = 0; i < sizeof(bitmap_pairs) / sizeof(bitmap_pair); i++)
    {
      int16_t x = -60;
      int16_t y = -60;
      for (uint16_t j = 0; j < 10; j++)
      {
        epd.writeScreenBuffer(); // use default for white
        epd.writeImage(bitmap_pairs[i].black, bitmap_pairs[i].red, x, y, 200, 200, false, false, true);
        epd.refresh();
        delay(1000);
        x += 40;
        y += 40;
        if ((x >= int16_t(epd.epd2.WIDTH)) || (y >= int16_t(epd.epd2.HEIGHT))) break;
      }
    }
    epd.writeScreenBuffer(); // use default for white
    epd.writeImage(bitmap_pairs[0].black, bitmap_pairs[0].red, 0, 0, 200, 200, false, false, true);
    epd.writeImage(bitmap_pairs[0].black, bitmap_pairs[0].red, int16_t(epd.epd2.WIDTH) - 200, int16_t(epd.epd2.HEIGHT) - 200, 200, 200, false, false, true);
    epd.refresh();
    delay(2000);
  }
}
#endif

#ifdef _GxBitmaps3c104x212_H_
void drawBitmaps3c104x212()
{
#if !defined(__AVR)
  bitmap_pair bitmap_pairs[] =
  {
    {Bitmap3c104x212_1_black, Bitmap3c104x212_1_red},
    {Bitmap3c104x212_2_black, Bitmap3c104x212_2_red},
    {WS_Bitmap3c104x212_black, WS_Bitmap3c104x212_red}
  };
#else
  bitmap_pair bitmap_pairs[] =
  {
    {Bitmap3c104x212_1_black, Bitmap3c104x212_1_red},
    //{Bitmap3c104x212_2_black, Bitmap3c104x212_2_red},
    {WS_Bitmap3c104x212_black, WS_Bitmap3c104x212_red}
  };
#endif
  if (epd.epd2.panel == GxEPD2::GDEW0213Z16)
  {
    for (uint16_t i = 0; i < sizeof(bitmap_pairs) / sizeof(bitmap_pair); i++)
    {
      epd.fillScreen(GxEPD_WHITE);
      epd.drawInvertedBitmap(0, 0, bitmap_pairs[i].black, epd.epd2.WIDTH, epd.epd2.HEIGHT, GxEPD_BLACK);
      if (bitmap_pairs[i].red == WS_Bitmap3c104x212_red)
      {
        epd.drawInvertedBitmap(0, 0, bitmap_pairs[i].red, epd.epd2.WIDTH, epd.epd2.HEIGHT, GxEPD_RED);
      }
      else epd.drawBitmap(0, 0, bitmap_pairs[i].red, epd.epd2.WIDTH, epd.epd2.HEIGHT, GxEPD_RED);
      emw3.blockedDisplay();
      delay(2000);
    }
  }
}
#endif

#ifdef _GxBitmaps3c128x296_H_
void drawBitmaps3c128x296()
{
#if !defined(__AVR)
  bitmap_pair bitmap_pairs[] =
  {
    {Bitmap3c128x296_1_black, Bitmap3c128x296_1_red},
    {Bitmap3c128x296_2_black, Bitmap3c128x296_2_red},
    {WS_Bitmap3c128x296_black, WS_Bitmap3c128x296_red}
  };
#else
  bitmap_pair bitmap_pairs[] =
  {
    //{Bitmap3c128x296_1_black, Bitmap3c128x296_1_red},
    //{Bitmap3c128x296_2_black, Bitmap3c128x296_2_red},
    {WS_Bitmap3c128x296_black, WS_Bitmap3c128x296_red}
  };
#endif
  if (epd.epd2.panel == GxEPD2::GDEW029Z10)
  {
    for (uint16_t i = 0; i < sizeof(bitmap_pairs) / sizeof(bitmap_pair); i++)
    {
      epd.fillScreen(GxEPD_WHITE);
      epd.drawInvertedBitmap(0, 0, bitmap_pairs[i].black, epd.epd2.WIDTH, epd.epd2.HEIGHT, GxEPD_BLACK);
      if (bitmap_pairs[i].red == WS_Bitmap3c128x296_red)
      {
        epd.drawInvertedBitmap(0, 0, bitmap_pairs[i].red, epd.epd2.WIDTH, epd.epd2.HEIGHT, GxEPD_RED);
      }
      else epd.drawBitmap(0, 0, bitmap_pairs[i].red, epd.epd2.WIDTH, epd.epd2.HEIGHT, GxEPD_RED);
      emw3.blockedDisplay();
      delay(2000);
    }
  }
}
#endif

#ifdef _GxBitmaps3c176x264_H_
void drawBitmaps3c176x264()
{
  bitmap_pair bitmap_pairs[] =
  {
    {Bitmap3c176x264_black, Bitmap3c176x264_red}
  };
  if (epd.epd2.panel == GxEPD2::GDEW027C44)
  {
    for (uint16_t i = 0; i < sizeof(bitmap_pairs) / sizeof(bitmap_pair); i++)
    {
      epd.fillScreen(GxEPD_WHITE);
      epd.drawBitmap(0, 0, bitmap_pairs[i].black, epd.epd2.WIDTH, epd.epd2.HEIGHT, GxEPD_BLACK);
      epd.drawBitmap(0, 0, bitmap_pairs[i].red, epd.epd2.WIDTH, epd.epd2.HEIGHT, GxEPD_RED);
      emw3.blockedDisplay();
      delay(2000);
    }
  }
}
#endif

#ifdef _GxBitmaps3c400x300_H_
void drawBitmaps3c400x300()
{
#if !defined(__AVR)
  bitmap_pair bitmap_pairs[] =
  {
    {Bitmap3c400x300_1_black, Bitmap3c400x300_1_red},
    {Bitmap3c400x300_2_black, Bitmap3c400x300_2_red},
    {WS_Bitmap3c400x300_black, WS_Bitmap3c400x300_red}
  };
#else
  bitmap_pair bitmap_pairs[] = {}; // not enough code space
#endif
  if (epd.epd2.panel == GxEPD2::GDEW042Z15)
  {
    for (uint16_t i = 0; i < sizeof(bitmap_pairs) / sizeof(bitmap_pair); i++)
    {
      epd.fillScreen(GxEPD_WHITE);
      epd.drawInvertedBitmap(0, 0, bitmap_pairs[i].black, epd.epd2.WIDTH, epd.epd2.HEIGHT, GxEPD_BLACK);
      epd.drawInvertedBitmap(0, 0, bitmap_pairs[i].red, epd.epd2.WIDTH, epd.epd2.HEIGHT, GxEPD_RED);
      emw3.blockedDisplay();
      delay(2000);
    }
  }
}
#endif

void drawBitmaps()
{
  epd.setFullWindow();
#ifdef _GxBitmaps104x212_H_
  drawBitmaps104x212();
#endif
#ifdef _GxBitmaps128x250_H_
  drawBitmaps128x250();
#endif
#ifdef _GxBitmaps128x296_H_
  drawBitmaps128x296();
#endif
#ifdef _GxBitmaps176x264_H_
  drawBitmaps176x264();
#endif
#ifdef _GxBitmaps400x300_H_
  drawBitmaps400x300();
#endif
#ifdef _GxBitmaps640x384_H_
  drawBitmaps640x384();
#endif
#ifdef _WS_Bitmaps800x600_H_
  drawBitmaps800x600();
#endif
  // 3-color
#ifdef _GxBitmaps3c104x212_H_
  drawBitmaps3c104x212();
#endif
#ifdef _GxBitmaps3c128x296_H_
  drawBitmaps3c128x296();
#endif
#ifdef _GxBitmaps3c176x264_H_
  drawBitmaps3c176x264();
#endif
#ifdef _GxBitmaps3c400x300_H_
  drawBitmaps3c400x300();
#endif
  // show these after the specific bitmaps
#ifdef _GxBitmaps200x200_H_
  drawBitmaps200x200();
#endif
  // 3-color
#ifdef _GxBitmaps3c200x200_H_
  drawBitmaps3c200x200();
#endif
}

void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("setup");
  delay(100);
  emw3.begin(nullptr);
  // first update should be full refresh
  helloWorld();
  delay(1000);
  // partial refresh mode can be used to full screen,
  // effective if display panel hasFastPartialUpdate
  helloFullScreenPartialMode();
  delay(1000);
  helloArduino();
  delay(1000);
  helloEpaper();
  delay(1000);
  //helloValue(123.9, 1);
  //delay(1000);
  showFont("FreeMonoBold9pt7b", &FreeMonoBold9pt7b);
  delay(1000);
  drawBitmaps();
  delay(100);
  if (epd.epd2.hasPartialUpdate)
  {
    showPartialUpdate();
    delay(1000);
  } // else // on GDEW0154Z04 only full update available, doesn't look nice
  //drawCornerTest();
  //showBox(16, 16, 48, 32, false);
  //showBox(16, 56, 48, 32, true);
  epd.powerOff();
  deepSleepTest();
  Serial.println("setup done");
  ESP.deepSleep(ESP.deepSleepMax());
}

void loop()
{
}