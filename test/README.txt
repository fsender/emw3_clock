
This directory is intended for PlatformIO Unit Testing and project tests.

Unit Testing is a software testing method by which individual units of
source code, sets of one or more MCU program modules together with associated
control data, usage procedures, and operating procedures, are tested to
determine whether they are fit for use. Unit testing finds problems early
in the development cycle.

More information about PlatformIO Unit Testing:
- https://docs.platformio.org/page/plus/unit-testing.html

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
 * 重要开发事项: 
 * 由于最新版软件和硬件原因,SD卡不再支持热插拔
 * 使用前,请修改 ESP8266SdFat 库中 ENABLE_DEDICATED_SPI 的值为 0
 * 不要使用 clearScreen() 函数, 请使用 fillScreen() 函数, 然后 performDisplay();

以下代码将会被emw3库使用,请在任何程序中都要包含以下代码.
注意最后一个 busy 引脚的参数为 -1, 因为我们使用中断来检测此功能
 GxEPD2_BW<GxEPD2_213, GxEPD2_213::HEIGHT> 
epd(GxEPD2_213(/*CS*/ _DEFINA_EPD_CS_PIN, /*DC*/ _DEFINA_EPD_DC_PIN,
/*RST*/ _DEFINA_EPD_RST_PIN, /*BUSY*/ -1));

U8G2_FOR_ADAFRUIT_GFX cn;



以下是我的设置方案
Below is my settings-----
第一步: 前往 GxEPD2_BW.cpp文件的111行, 然后用以下代码替换原来的switch代码
Step 1: Go to file GxEPD2_BW.h line 111:
replace to the code below:
      switch (getRotation())
      {
        case 1:
          _swap_(x, y);
          x = 121 - x;
          break;
        case 2:
          x = 121 - x;
          y = HEIGHT - y - 1;
          break;
        case 3:
          _swap_(x, y);
          y = HEIGHT - y - 1;
          break;
      }
第二步: 前往 GxEPD2_213.cpp文件的322行, 然后用以下代码替换原来的代码
Step 2: Go to file GxEPD2_213.cpp line 322:
replace to the code below:

void GxEPD2_213::_InitDisplay()
{
  if (_hibernating) _reset();
  _writeCommand(0x01); // Panel configuration, Gate selection
  _writeData((HEIGHT - 1) % 256);
  _writeData((HEIGHT - 1) / 256);
  _writeData(0x00);
  _writeCommand(0x03); // Set Gate driving voltage
  _writeData(0x15);
  _writeData(0x0e);
  _writeCommand(0x04); // Set Source output voltage.
  _writeData(0x1d);

  _writeCommand(0x0c); // softstart
  _writeData(0xd7);
  _writeData(0xd6);
  _writeData(0x9d);
  _writeCommand(0x2d); // VCOM setting
  _writeData(0x55);    // * different
  _writeCommand(0x3a); // DummyLine
  _writeData(0x1a);    // 4 dummy line per gate
  _writeCommand(0x3b); // Gatetime
  _writeData(0x08);    // 2us per line
  _setPartialRamArea(0, 0, WIDTH, HEIGHT);
}
const uint8_t GxEPD2_213::LUTDefault_full[] PROGMEM = { 0x32,  // command
  0x22, 0x55, 0xaa, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x0f, 0x0f, 0x13, 0x13, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

const uint8_t GxEPD2_213::LUTDefault_part[] PROGMEM = { 0x32,  // command
  0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x0d, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  };

第三步: 前往 Adafruit_GFX.cpp文件的大约111行, 找到Adafruit_GFX::Adafruit_GFX(w, h)函数, 
然后用以下代码替换原来的代码 (行数不同因为库版本不同)
Step 3: Go to file Adafruit_GFX.cpp around line 111:
Go to function Adafruit_GFX::Adafruit_GFX(int16_t w, int16_t h)
replace to the code below: (different version of libraries have different position.)

from raw:  _width = WIDTH;
to------>  _width = WIDTH - 6;


第四步: 前往 Adafruit_GFX.cpp文件的大约1316行, 找到Adafruit_GFX::setRotation(uint8_t x)函数, 
然后用以下代码替换原来的代码
Step 4: Go to file Adafruit_GFX.cpp around line 1316:
Go to function Adafruit_GFX::setRotation(uint8_t x)
replace to the code below: (different version of libraries have different position.)

void Adafruit_GFX::setRotation(uint8_t x) {
  rotation = (x & 3);
  switch (rotation) {
  case 0:
  case 2:
    _width = WIDTH - 6;
    _height = HEIGHT;
    break;
  case 1:
  case 3:
    _width = HEIGHT;
    _height = WIDTH - 6;
    break;
  }
}
第五步: 前往文件 GxEPD2_213.h, 行30, 31:
把后面的两个初始值修改为0
Step 5: Go to file GxEPD2_213.h line 30 to 31:
Set these two constants to 0 :

    static const uint16_t full_refresh_time = 0; // ms, e.g. 3883686us
    static const uint16_t partial_refresh_time = 0; // ms, e.g. 268173us