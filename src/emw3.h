/**
 * @file emw3.h
 * @author fsender (f_ender@163.com)
 * @brief emw3 brick development class.
 * @version 0.0.1
 * @date 2021-10-21
 */

#ifndef _F_ENDER_EMW3_H
#define _F_ENDER_EMW3_H
#include <Arduino.h>
#include <SD.h> //可能类内调用SD卡或者SPIFFS文件系统,
#include <FS.h> //用于梳理启动文件,或者数据加载等功能
#include <ESP8266SDUpdater.h>

#include "emw3_definas.h"


typedef enum _f_ender_Btn{
  BtnL = _DEFINA_BtnL,BtnM = _DEFINA_BtnM,BtnR = _DEFINA_BtnR
} Btn;

class emw3_base{
  public:
    emw3_base();
    void begin();
    uint8_t getBtn(Btn btn);
    uint8_t needSDBegin();
    uint8_t needEPDBegin();
  protected:
  private:
    uint8_t BtnL_lastpress = 0;
    uint8_t BtnM_lastpress = 0;
    uint8_t BtnR_lastpress = 0;
};

#endif