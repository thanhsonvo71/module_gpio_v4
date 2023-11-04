#pragma mark - Depend ESP8266Audio and ESP8266_Spiram libraries
/* 
code    color
0x0000  Black
0xFFFF  White
0xBDF7  Light Gray
0x7BEF  Dark Gray
0xF800  Red
0xFFE0  Yellow
0xFBE0  Orange
0x79E0  Brown
0x7E0   Green
0x7FF   Cyan
0x1F    Blue
0xF81F  Pink
*/
#include "config.h"
#include "image.h"
#include "image2.h"
//#include "sound.h"
#include "sound2.h"
#include "wifi.h"
#include "wifi_off.h"
#include "AudioFileSourcePROGMEM.h"
#include "AudioFileSourceID3.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2S.h"
#include "AudioGeneratorAAC.h"
#include "AudioFileSourcePROGMEM.h"

#define Bit_Clock_BCLK 26
#define Word_Select_WS 25
#define Serial_Data_SD 33
#define GAIN 1.0
//
#include <I2S.h>
#include <WiFi.h>
#include <EEPROM.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include <string.h>
#include "/home/hitech/Desktop/Son_setting/arduino/code/src/string_Iv2/string_Iv2.h"
// wifi to connect
char * ssid="MVP_PRO";
char * password="MarueiPRO&@1";
//
AudioFileSourcePROGMEM *in;
AudioGeneratorAAC *aac;
AudioOutputI2S *out;
WiFiClient client;
HTTPClient http;
long time_out_wifi;
double battery_capacity=350;
double battery_capacity_now;
double battery_capacity_shutdown=120;
long  current_charge,current_discharge;
int status_wifi;
String http_respone="";
int frist_time_update=0;
string date;
// http var
string data_httppost="\"name_seri\":\"WATCH_2\"";
const char* serverName = "http://10.0.70.20/test3.php";
int num_input_gpio=8,num_output_gpio=8;
vector<int> input_user_status;
vector<int> output_user_status;
class other_module_gpio{
  public:
    string name_seri;
    vector<int> input_user_status;
    vector<int> output_user_status;
};
int packing[2];
int waiting[2];
vector<other_module_gpio> my_other_module_gpio;
long time_motor=0;
int request_music=0;
long time_gpio=0;
// watch class
TTGOClass *ttgo;
static uint8_t conv2d(const char *p)
{
    uint8_t v = 0;
    if ('0' <= *p && *p <= '9')
        v = *p - '0';
    return 10 * v + *++p - '0';
}
//uint64_t h = conv2d(__TIME__), m = conv2d(__TIME__ + 3), s = conv2d(__TIME__ + 6); 
uint64_t time_before_update;
uint64_t h = 0, m = 0, s = 0; 
uint64_t h_set=0,m_set=0,s_set=0;
// 
int16_t time_to_sleep=10000;
int irq=0;
int watch_unsleep=1;
static SemaphoreHandle_t xSemaphores = NULL;
//
string input_to_json(){
  static string value_return;
  value_return="";
  value_return=value_return+",\"num_in\":\""+to_string(num_input_gpio)+"\"";
  for(int i=0;i<input_user_status.size();i++){
    value_return=value_return+",\"in"+to_string(i+1)+"\":\""+to_string(input_user_status[i])+"\"";
  }
  return value_return;
}
string output_to_json(){
  static string value_return;
  value_return="";
  value_return=value_return+",\"num_out\":\""+to_string(num_output_gpio)+"\"";
  for(int i=0;i<output_user_status.size();i++){
    value_return=value_return+",\"out"+to_string(i+1)+"\":\""+to_string(output_user_status[i])+"\"";
  }
  return value_return;
}
string battery_to_json(){
  static string value_return;
  value_return="";
  value_return=value_return+",\"battery\":\"";
  value_return=value_return+"capacity:"+to_string(battery_capacity)+"mA|";
  value_return=value_return+"capacity_now:"+to_string(battery_capacity_now)+"mA|";
  value_return=value_return+"voltage:"+to_string(ttgo->power->getBattVoltage())+"mV|";
  value_return=value_return+"current_charge:"+to_string(current_charge)+"mA|";
  value_return=value_return+"current_discharge:"+to_string(current_discharge)+"mA|";
  value_return=value_return+"soc1:"+to_string(ttgo->power->getBattPercentage())+"%|";
  value_return=value_return+"soc2:"+to_string((long)(100*((battery_capacity_now-battery_capacity_shutdown)/(battery_capacity-battery_capacity_shutdown))))+"%|";
  value_return=value_return+"\"";
  return value_return;
}
//
void ouput_user_set_string(string data){
  static string_Iv2 data1;
  data1.detect(data,"","||","");
  if(data1.data1.size()>=2){
      static string_Iv2 data2;
      data2.detect(data1.data1[1],"(","|",")");
      for(int i=0;i<data2.data1.size();i++){
          if(stoi(data2.data1[i]) < num_output_gpio){
              output_user_status[stoi(data2.data1[i])]=stoi(data2.data2[i]);
          }
      }
  }
  Serial.println(data.c_str());
}
void output_table(string data){
  static string_Iv2 data1;
  data1.detect(data,"(","",")");
  for(int i=0;i<data1.data1.size();i++){
      //
      static string_Iv2 data2;
      data2.detect(data1.data1[i],"","|","");
      static int is_have;
      is_have=-1;
      //
      for(int j=0;j<my_other_module_gpio.size();j++){
        if(data2.data1[0]==my_other_module_gpio[j].name_seri){
          is_have=j;
          break;
        }
      }
      //
      if(is_have==-1){
        my_other_module_gpio.resize(my_other_module_gpio.size()+1);
        my_other_module_gpio[my_other_module_gpio.size()-1].name_seri=data2.data1[0];
      }else{
        data2.detect(data2.data1[1],"~","=","~");
        my_other_module_gpio[is_have].output_user_status.resize(data2.data1.size());
        for(int k=0;k<data2.data1.size();k++){
          my_other_module_gpio[is_have].output_user_status[k]=stoi(data2.data2[k]);
        }
      }
  }
  
}
void input_table(string data){
  static string_Iv2 data1;
  data1.detect(data,"(","",")");
  for(int i=0;i<data1.data1.size();i++){
      //
      static string_Iv2 data2;
      data2.detect(data1.data1[i],"","|","");
      static int is_have;
      is_have=-1;
      //
      for(int j=0;j<my_other_module_gpio.size();j++){
        if(data2.data1[0]==my_other_module_gpio[j].name_seri){
          is_have=j;
          break;
        }
      }
      //
      if(is_have==-1){
        my_other_module_gpio.resize(my_other_module_gpio.size()+1);
        my_other_module_gpio[my_other_module_gpio.size()-1].name_seri=data2.data1[0];
      }else{
        data2.detect(data2.data1[1],"~","=","~");
        my_other_module_gpio[is_have].input_user_status.resize(data2.data1.size());
        for(int k=0;k<data2.data1.size();k++){
          my_other_module_gpio[is_have].input_user_status[k]=stoi(data2.data2[k]);
        }
      }
  }
  
}
void process_data(string name,string data){
  if(name=="output_user_set_string") ouput_user_set_string(data);
  if(name=="output_table") output_table(data);
  if(name=="input_table")  input_table(data);
  if(name=="date") date=data; 
  if(frist_time_update==0){
    if(name=="time_h") h_set=stoi(data);
    if(name=="time_m") m_set=stoi(data);
    if(name=="time_s") s_set=stoi(data);
    time_before_update=millis();
  }
}
void process_time(){
  h=(uint64_t)(h_set+(uint64_t)(m_set*60+s_set+((millis()-time_before_update)/1000))/3600)%24;
  m=(uint64_t)(m_set+(uint64_t)(s_set+(millis()-time_before_update)/(1000))/60)%60;
  s=(uint64_t)(s_set+(uint64_t)((millis()-time_before_update)/(1000))%60);
}
void draw_time(uint16_t x, uint16_t y){
  static int i=0;
  //
  ttgo->tft->setTextColor(0x7E0, TFT_BLACK);
  ttgo->tft->drawNumber((uint8_t)(h/10), x, y, 7);
  ttgo->tft->drawNumber(h-(uint8_t)(h/10)*10, x+40, y, 7);
  ttgo->tft->drawNumber((uint8_t)(m/10), x+100, y, 7);
  ttgo->tft->drawNumber(m-(uint8_t)(m/10)*10, x+140, y, 7);
  if(i==0){
      i=1;
      ttgo->tft->setTextColor(0x7E0, TFT_BLACK);
      ttgo->tft->drawString(":",x+85,y,7);
  }else{
      ttgo->tft->setTextColor(0x7BEF, TFT_BLACK);
      ttgo->tft->drawString(":",x+85,y,7);
      i=0;
  }
  //
  ttgo->tft->setTextColor(TFT_GREEN, TFT_BLACK);
  
  ttgo->tft->drawString(date.c_str(),x+50,y+55,2); 
}
void draw_image(uint16_t x, uint16_t y, int mode){
  if(mode){
    ttgo->tft->pushImage(x,y,56, 66,image);
  }
  else{
    ttgo->tft->pushImage(x,y,56, 66,image2);
  }
}
void draw_packing_waiting(uint16_t x, uint16_t y ,int d,int c, int b, int a){
  //
  ttgo->tft->setTextColor(TFT_YELLOW, TFT_BLUE);
  ttgo->tft->drawString("Waiting",x-30,y+80,2);
  ttgo->tft->drawString("Parking",x-30,y+105,2);
  //
  static uint16_t color_red=0xF800,color_green=0x7E0,color_gray=0x7BEF;
  //
  if(a==1) ttgo->tft->fillRoundRect(x+25,y+78,20,20,3,color_green);
  else ttgo->tft->fillRoundRect(x+25,y+78,20,20,3,color_gray);
  if(b==1) ttgo->tft->fillRoundRect(x+55,y+78,20,20,3,color_green);
  else ttgo->tft->fillRoundRect(x+55,y+78,20,20,3,color_gray);
  //
  if(c==1) ttgo->tft->fillRoundRect(x+25,y+103,20,20,3,color_red);
  else ttgo->tft->fillRoundRect(x+25,y+103,20,20,3,color_gray);
  if(d==1) ttgo->tft->fillRoundRect(x+55,y+103,20,20,3,color_red);
  else ttgo->tft->fillRoundRect(x+55,y+103,20,20,3,color_gray);

}
void draw_wifi(uint16_t x,uint16_t y, int mode){
  if(mode){
    ttgo->tft->pushImage(x,y,24, 22 ,wifi);
  }
  else{
    ttgo->tft->pushImage(x,y,24, 22 ,wifi_off);
  }
}
void draw_battery(uint16_t x, uint16_t y){
  //
  static int soc;
  //soc=(long)ttgo->power->getBattPercentage();
  soc=(long)(100*((battery_capacity_now-battery_capacity_shutdown)/(battery_capacity-battery_capacity_shutdown)));
  //
  current_charge=ttgo->power->getBattChargeCurrent();
  current_discharge=ttgo->power->getBattDischargeCurrent();
  static uint16_t color;
  if(soc > 70) color=0x7E0;
  else if(soc<=70 & soc > 20) color=0xFFE0;
  else color=0xF800;
  //
  static uint16_t color_back=0x0000;
  ttgo->tft->fillRect(x-165,y,200,15,color_back);
  if(ttgo->power->isVBUSPlug()){
    static int num;
    color=0x7E0;
    //
    ttgo->tft->setTextColor(color, TFT_BLACK);
    ttgo->tft->drawString(String((long)current_charge)+"mA",x-165,y,2);
    //
    ttgo->tft->setTextColor(color, TFT_BLACK);
    ttgo->tft->drawString(String((long)battery_capacity_now)+"mA",x-100,y,2);
    //
    ttgo->tft->setTextColor(color, TFT_BLACK);
    ttgo->tft->drawString(String(soc)+"%",x-40,y,2);
    // bounder
    ttgo->tft->drawRoundRect(x,y,26,15,3,color);
    ttgo->tft->fillRect(x-3,y+4,3,7,color);
    // cell
    if(num >= 3) ttgo->tft->fillRect(x+2,y+2,6,11,color);
    else ttgo->tft->fillRect(x+2,y+2,6,11,color_back);
    if(num >= 2) ttgo->tft->fillRect(x+2+8,y+2,6,11,color);
    else ttgo->tft->fillRect(x+2+8,y+2,6,11,color_back);
    if(num >= 1) ttgo->tft->fillRect(x+2+16,y+2,6,11,color);
    else ttgo->tft->fillRect(x+2+16,y+2,6,11,color_back);
    num++;
    if(num>=4) num=0;
  }else{
    //
    ttgo->tft->setTextColor(color, TFT_BLACK);
    ttgo->tft->drawString(String((long)(-current_discharge))+"mA",x-165,y,2);
    //
    ttgo->tft->setTextColor(color, TFT_BLACK);
    ttgo->tft->drawString(String((long)battery_capacity_now)+"mA",x-100,y,2);
    //
    ttgo->tft->setTextColor(color, TFT_BLACK);
    ttgo->tft->drawString(String(soc)+"%",x-40,y,2);
    // bounder
    ttgo->tft->drawRoundRect(x,y,26,15,3,color);
    ttgo->tft->fillRect(x-3,y+4,3,7,color);
    // cell
    if(soc >= 67) ttgo->tft->fillRect(x+2,y+2,6,11,color);
    else ttgo->tft->fillRect(x+2,y+2,6,11,color_back);
    if(soc >= 33)ttgo->tft->fillRect(x+2+8,y+2,6,11,color);
    else ttgo->tft->fillRect(x+2+8,y+2,6,11,color_back);
    ttgo->tft->fillRect(x+2+16,y+2,6,11,color);
  }
}
void inti_draw_clock(int r, int x, int y){
  // Draw clock face
  float sx,sy,x0,yy0,x1,y1,yy1;
  ttgo->tft->fillCircle(x, y, r+2, 0xFFE0);
  ttgo->tft->fillCircle(x, y, r-2, TFT_BLACK);
  // Draw 12 lines
  for (int i = 0; i < 360; i += 30) {
      sx = cos((i - 90) * 0.0174532925);
      sy = sin((i - 90) * 0.0174532925);
      x0 = sx * (r) + x;
      yy0 = sy * (r) + y;
      x1 = sx * (r-8) + x;
      yy1 = sy * (r-8) + y;
      ttgo->tft->drawLine(x0, yy0, x1, yy1, TFT_RED);
  }
  // Draw 60 dots
  for (int i = 0; i < 360; i += 6) {
      sx = cos((i - 90) * 0.0174532925);
      sy = sin((i - 90) * 0.0174532925);
      x0 = sx * (r-5) + x;
      yy0 = sy * (r-5) + y;
      // Draw minute markers
      ttgo->tft->drawPixel(x0, yy0, TFT_WHITE);

      // Draw main quadrant dots
      if (i == 0 || i == 180) ttgo->tft->fillCircle(x0, yy0, 2, TFT_WHITE);
      if (i == 90 || i == 270) ttgo->tft->fillCircle(x0, yy0, 2, TFT_WHITE);
  }

}
void draw_clock(int r, int x, int y){
    static float sdeg,mdeg,hdeg;
    static float hx,hy,mx,my,sx,sy;
    static float ohx,ohy,omx,omy,osx,osy;
    // Pre-compute hand degrees, x & y coords for a fast screen update
    sdeg = (float)s * 6;                // 0-59 -> 0-354
    mdeg = (float)m * 6 + sdeg * 0.01666667; // 0-59 -> 0-360 - includes seconds
    hdeg = (float)(h%12) * 30 + mdeg * 0.0833333; // 0-11 -> 0-360 - includes minutes and seconds
    hx = cos((hdeg - 90) * 0.0174532925);
    hy = sin((hdeg - 90) * 0.0174532925);
    mx = cos((mdeg - 90) * 0.0174532925);
    my = sin((mdeg - 90) * 0.0174532925);
    sx = cos((sdeg - 90) * 0.0174532925);
    sy = sin((sdeg - 90) * 0.0174532925);
    // hours
    ttgo->tft->drawLine(ohx, ohy, x, y+1, TFT_BLACK);
    ohx = hx * r*0.5 + x;
    ohy = hy * r*0.5 + y;
    ttgo->tft->drawLine(ohx, ohy, x, y+1, TFT_WHITE);
    // minute
    ttgo->tft->drawLine(omx, omy, x, y+1, TFT_BLACK);
    omx = mx * r*0.7 + x;
    omy = my * r*0.7 + y;
    ttgo->tft->drawLine(omx, omy, x, y+1, TFT_WHITE);
    // second
    ttgo->tft->drawLine(osx, osy, x, y+1, TFT_BLACK);
    osx = sx * r *0.8 + x;
    osy = sy * r* 0.8 + y;
    ttgo->tft->drawLine(osx, osy, x, y+1, TFT_RED);
    // 
    ttgo->tft->fillCircle(x, y+1, 3, TFT_RED);
}
void thread1(void *args)
{
    for (;;) {
        if ( xSemaphoreTake( xSemaphores, ( TickType_t ) 10 ) == pdTRUE ) {
                static uint64_t time_num=millis();
                if(millis()-time_num >=1000){
                    s=s+(long)((millis()-time_num)/1000);
                    process_time();
                    if(watch_unsleep==0) ttgo->closeBL();
                    else{
                      draw_time(60,30);
                      draw_clock(55,180,170);  
                      //draw_image(35,100,1);
                      draw_battery(210,0);
                      draw_wifi(10,0,status_wifi);
                      draw_packing_waiting(35,100,packing[0],packing[1],waiting[0],waiting[1]);
                      ttgo->openBL();
                    }
                    time_num=millis();   
                }
           xSemaphoreGive( xSemaphores );
        }
        delay(10);
    }
}
void thread2(void *args)
{
    for (;;) {
        if ( xSemaphoreTake( xSemaphores, ( TickType_t ) 10 ) == pdTRUE ) {
                //
                static uint64_t time_num=millis();
                if(millis()-time_num >=100){
                    if(irq==1){
                        irq=0;
                        ttgo->power->clearIRQ();
                        if(time_motor>0){
                            time_motor=0;
                            request_music=0;
                        }else{
                            if(watch_unsleep==0){
                                time_to_sleep=10000;                    
                                watch_unsleep=1;
                            }else{
                              time_to_sleep=0;
                              watch_unsleep=0;
                            }
                        }
                    }
                    static int16_t x,y;
                    if (ttgo->getTouch(x, y)) {
                          time_to_sleep=10000;
                          if(watch_unsleep==0) {
                            watch_unsleep=1;
                          }
                    }
                    //
                    if(time_to_sleep>0) time_to_sleep=time_to_sleep-100;//(millis()-time_num);
                    if(time_to_sleep<=0) {
                        time_to_sleep=0;
                        watch_unsleep=0;
                    }
                    if(time_motor>0){
                        time_to_sleep=10000;                    
                        watch_unsleep=1;
                    }
                    //
                    static long time_action_motor;
                    static int action_motor;
                    if(time_motor>0){
                      time_motor=time_motor-100;//(millis()-time_num);
                      if(time_motor<=0){
                        time_motor=0;
                        request_music=0;
                      }
                      //
                      time_action_motor=time_action_motor-100;//(millis()-time_num);
                      if(time_action_motor<=0){
                        if(action_motor==1){
                          action_motor=0;
                          time_action_motor=1000;
                        }else{
                          action_motor=1;
                          time_action_motor=1500;
                        }
                      }
                      if(action_motor) ttgo->motor->onec();
                    }else{
                      time_action_motor=1500;
                      action_motor=1;
                    }
                    //
                    if(time_gpio>0){
                      time_gpio=time_gpio-100;//(millis()-time_num);
                      if(time_gpio<=0){
                        time_gpio=0;
                        output_user_status[0]=0;
                      }
                    }
                    //
                    current_charge=(long)ttgo->power->getBattChargeCurrent();
                    current_discharge=(long)ttgo->power->getBattDischargeCurrent();
                    static int num_full_battery=0;
                    if(ttgo->power->isVBUSPlug() & current_charge <=0) num_full_battery++;
                    else num_full_battery=0;
                    
                    if(current_charge>0){
                      battery_capacity_now=battery_capacity_now+(double)current_charge*0.1/3600;
                      if(battery_capacity_now>battery_capacity) battery_capacity_now=battery_capacity;
                    }
                    else{
                      battery_capacity_now=battery_capacity_now-(double)current_discharge*0.1/3600;
                      if(battery_capacity_now<battery_capacity_shutdown) battery_capacity_now=battery_capacity_shutdown;
                    }
                    if(num_full_battery>=100) {
                      battery_capacity_now=battery_capacity;
                      num_full_battery=100;
                    }
                    //
                    static uint8_t A,B,C,D;
                    A=(uint8_t)(battery_capacity_now/100);
                    B=(uint8_t)(battery_capacity_now-A*100);
                    C=(uint8_t)((battery_capacity_now-A*100-B)*100);
                    EEPROM.write(0,A);
                    EEPROM.write(1,B);
                    EEPROM.write(2,C);
                    EEPROM.commit();
                    //
                    time_num=millis();
                }
                xSemaphoreGive( xSemaphores );
        }
        
        delay(10);
    }
}
void thread3(void *args)
{
    static int request_music_static=0;
    static int count;
    for (;;) {
        A:
        if ( xSemaphoreTake( xSemaphores, ( TickType_t ) 10 ) == pdTRUE ) {
           //
           static uint64_t time_num=millis();
                if(millis()-time_num >=500){
                      time_num=millis();
                      if(request_music){
                          if(!request_music_static){
                              delete in;
                              delete aac;
                              delete out;
                              //
                              in = new AudioFileSourcePROGMEM(buzze, sizeof(buzze));
                              aac = new AudioGeneratorAAC();
                              out = new AudioOutputI2S();
                              out -> SetGain(GAIN);
                              out -> SetPinout(Bit_Clock_BCLK,Word_Select_WS,Serial_Data_SD);
                              aac->begin(in, out);
                              count=0;
                              request_music_static=1;
                              //ttgo->enableLDO3();
                          }
                      }
                      else {
                        request_music_static=0;
                      }
                }
           xSemaphoreGive( xSemaphores );
        }
        while(request_music_static==1){
          count++;
          if(!aac->loop()){
              delete in;
              delete aac;
              delete out;
              //
              in = new AudioFileSourcePROGMEM(buzze, sizeof(buzze));
              aac = new AudioGeneratorAAC();
              out = new AudioOutputI2S();
              out -> SetGain(GAIN);
              out -> SetPinout(Bit_Clock_BCLK,Word_Select_WS,Serial_Data_SD);
              aac->begin(in, out);
              count=0;
          }
          if(count>=2000){
            goto A;
            count=0;
          }
        }
        delay(10);
    }
}
void setup(void)
{ 
      //
      Serial.begin(115200);
      ttgo = TTGOClass::getWatch();
      ttgo->begin();
      // sound
      ttgo->enableLDO3();
      in = new AudioFileSourcePROGMEM(buzze, sizeof(buzze));
      aac = new AudioGeneratorAAC();
      out = new AudioOutputI2S();
      out -> SetGain(GAIN);
      out -> SetPinout(Bit_Clock_BCLK,Word_Select_WS,Serial_Data_SD);
      aac->begin(in, out);
      //      
      ttgo->motor_begin();
      //
      ttgo->openBL();
      ttgo->tft->fillScreen(TFT_BLACK);
      ttgo->tft->setTextColor(TFT_WHITE, TFT_BLACK);
      //
      pinMode(AXP202_INT, INPUT_PULLUP);
      pinMode(TOUCH_INT, INPUT);
      attachInterrupt(AXP202_INT, [] {
        irq = 1;
      }, FALLING);
      ttgo->power->enableIRQ(AXP202_PEK_SHORTPRESS_IRQ, true);
      ttgo->power->clearIRQ();
      //
      ttgo->tft->setSwapBytes(true);
      draw_image(35,100,1);
      draw_time(60,30);
      inti_draw_clock(55, 180, 170);
      draw_clock(55,180,170);
      draw_battery(210,0);
      draw_wifi(10,0,status_wifi);
      draw_packing_waiting(35,100,packing[0],packing[1],waiting[0],waiting[1]);
      ttgo->power->begin();
      ttgo->power->adc1Enable(
        AXP202_VBUS_VOL_ADC1 |
        AXP202_VBUS_CUR_ADC1 |
        AXP202_BATT_CUR_ADC1 |
        AXP202_BATT_VOL_ADC1,
        true);
      // Read battery from epprom
      static uint8_t A,B,C;
      EEPROM.begin(3);
      A=EEPROM.read(0);
      B=EEPROM.read(1);
      C=EEPROM.read(2);
      Serial.println(String(A)+"|"+String(B)+"|"+String(C));
      battery_capacity_now=(double)A*100+(double)B+(double)C/100;
      Serial.print("Capcatiy now: ");
      Serial.println(battery_capacity_now);
      // network 
      WiFi.begin(ssid, password);
      http.setTimeout(2000);
      input_user_status.resize(num_input_gpio);
      output_user_status.resize(num_output_gpio);
      //!Turn on the audio power
      xSemaphores = xSemaphoreCreateMutex();
      xTaskCreate(thread1, "thread1", 2048, NULL, 1, NULL);
      xTaskCreate(thread2, "thread2", 2048, NULL, 1, NULL);
      xTaskCreate(thread3, "thread3", 2048, NULL, 1, NULL);
      Serial.println("Set Up done\n");
}
void loop()
{   
  static int wifi_reconnect=0;
  static int http_post=0;
  if ( xSemaphoreTake( xSemaphores, ( TickType_t ) 10 ) == pdTRUE ) {
          static uint64_t time_num=millis();
          if(millis()-time_num >=2000){
              if(WiFi.status()== WL_CONNECTED){
                status_wifi=1;
                time_out_wifi=0;
                http_post=1;
              }else{
                time_out_wifi=time_out_wifi+1000;
                status_wifi=0;
                http_post=0;
              }
              if(time_out_wifi>=5000){
                  wifi_reconnect=1;
                  time_out_wifi=0;
              }
              time_num=millis();
          }
          //
          if(http_respone!=""){
            //Serial.println(http_respone.c_str());
            JSONVar myObject = JSON.parse(http_respone);
            JSONVar keys = myObject.keys();
            for (int i = 0; i < keys.length(); i++) {
                JSONVar value = myObject[keys[i]];
                process_data((string)keys[i],(string)value);
            }
            http_respone="";
            frist_time_update=1;
          }
          // misssion here
          static uint64_t time_num2=millis();
          if(millis()-time_num2 >=100){
              //
              static string name_module_define="IB06_916b";
              for(int i=0;i<my_other_module_gpio.size();i++){
                if(my_other_module_gpio[i].name_seri==name_module_define){
                    packing[0]=my_other_module_gpio[i].output_user_status[4];
                    packing[1]=my_other_module_gpio[i].output_user_status[6];
                    waiting[0]=my_other_module_gpio[i].output_user_status[3];
                    waiting[1]=my_other_module_gpio[i].output_user_status[5];
                }
              }
              // motor here
              static int f_out1,out1;
              f_out1=out1;
              out1=output_user_status[0];
              if(f_out1==0 & out1==1){
                time_motor=30000;
                request_music=1;                   
                watch_unsleep=1;
                time_gpio=5000;
              }
              time_num2=millis();
          }
      xSemaphoreGive( xSemaphores );
  }
  if(wifi_reconnect){
    wifi_reconnect=0;
    WiFi.reconnect();
  }
  if(http_post){
      http_post=0;
      //get json string here
      http.begin(client, serverName);
      http.addHeader("Content-Type", "application/json");
      static long httpResponseCode;
      static string request_string;
      request_string="{"+data_httppost+input_to_json()+output_to_json()+battery_to_json()+"}";
      //
      httpResponseCode= http.POST(request_string.c_str());
      if(httpResponseCode==200){
        http_respone = http.getString();
      }else{
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
      }
      // Free resources
      http.end();
  }
  delay(10);   
}
