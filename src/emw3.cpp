/**
 * @file emw3.cpp
 * @author fsender (f_ender@163.com)
 * @brief fill----------------------------<<<
 * @version 0.0.1
 * @date 2021-10-21
 */
#include "emw3.h"

emw3_base::emw3_base(){
  //fill soon
}
void emw3_base::begin(){
  //fill soon
}
/**
 * @brief 在下次填充屏幕或者调用SD卡时,允许使用 Btnx_lastpress 来
 * 确定是否进行额外的 begin 或者 init 函数, 有的按钮不影响此功能
 * 在现有硬件下,如果调用了BtnL的getBtn函数, 则下次使用屏幕时需要重
 * 新 init ,SD卡则为BtnL或者BtnM.
 * 
 * @param btn 需要读取的按钮
 * @return uint8_t 按钮电平值 0:LOW  1:HIGH
 */
uint8_t emw3_base::getBtn(Btn btn){
  //fill soon
  return 1;
}
/**
 * @brief 获取下次调用SD卡时是否需要再调用SD.begin()
 * 
 * @return uint8_t 0:不需要 No need   1:需要 Need
 */
uint8_t emw3_base::needSDBegin(){
  return 0;
}
uint8_t emw3_base::needEPDBegin(){
  return 0;
}