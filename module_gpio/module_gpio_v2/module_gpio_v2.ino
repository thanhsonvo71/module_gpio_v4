#define out1    23
#define out2    22
#define out3    21
#define out4    19
#define out5    17
#define out6    18
#define out7    5
#define out8    0

#define in1   32
#define in2   33
#define in3   25
#define in4   26
#define in5   27
#define in6   14
#define in7   12
#define in8   13

#include <WiFi.h>
#include <EEPROM.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include <string.h>
#include <TaskScheduler.h>
//
#include "/home/hitech/Desktop/Son_setting/arduino/code/src/string_Iv2/string_Iv2.h"
#include "/home/hitech/Desktop/Son_setting/arduino/code/src/stoi/stoi.h"
#include "/home/hitech/Desktop/Son_setting/arduino/code/src/stof/stof.h"
#include "/home/hitech/Desktop/Son_setting/arduino/code/src/mission/mission_init.h"
// wifi to connect
char * ssid="MVP_PRO";
char * password="MarueiPRO&@1";
static string data;
HTTPClient http;
WiFiClient client;
string data_httppost="\"name_seri\":\"IB06_916b\"";
String http_respone="";
const char* serverName = "http://10.0.70.20/test3.php";
//
static SemaphoreHandle_t xSemaphores = NULL;
int request_post=0;
//
int num_input_gpio=8,num_output_gpio=8;
// mission
static int frist_time_get_mission=1;
multiple_mission my_multiple_mission;
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
  return value_return;
}
string frist_time_get_mission_json(){
  static string value_return;
  value_return="";
  if(frist_time_get_mission==1){
    value_return=value_return+",\"frist_time_get_mission\":\""+to_string(1)+"\"";
  }
  return value_return;
}
string output_user_set_string_fesp_json(){
  static string value_return;
  value_return="";
  if(gpio_module_set!=""){
    value_return=value_return+",\"output_user_set_string_fesp\":\""+gpio_module_set+"\"";
    gpio_module_set="";
  }
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
          if(stoi(data2.data1[i]) < output_user_status.size()){
              output_user_status[stoi(data2.data1[i])]=stoi(data2.data2[i]);
              static string abc;
              abc="out"+to_string(stoi(data2.data1[i]))+"->"+to_string(stoi(data2.data2[i]));
              Serial.println(abc.c_str());
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
      for(int j=0;j<my_module.size();j++){
        if(data2.data1[0]==my_module[j].name){
          is_have=j;
          break;
        }
      }
      //
      if(is_have==-1){
        cout<<"Have module "<<data2.data1[0]<<endl;
        my_module.resize(my_module.size()+1);
        my_module[my_module.size()-1].name=data2.data1[0];
      }else{
        data2.detect(data2.data1[1],"~","=","~");
        my_module[is_have].output_user.resize(data2.data1.size());
        for(int k=0;k<data2.data1.size();k++){
          my_module[is_have].output_user[k]=stoi(data2.data2[k]);
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
      for(int j=0;j<my_module.size();j++){
        if(data2.data1[0]==my_module[j].name){
          is_have=j;
          break;
        }
      }
      //
      if(is_have==-1){
        cout<<"Have module "<<data2.data1[0]<<endl;
        my_module.resize(my_module.size()+1);
        my_module[my_module.size()-1].name=data2.data1[0];
      }else{
        data2.detect(data2.data1[1],"~","=","~");
        my_module[is_have].input_user.resize(data2.data1.size());
        for(int k=0;k<data2.data1.size();k++){
          my_module[is_have].input_user[k]=stoi(data2.data2[k]);
        }
      }
  }
  
}
void process_data(string name,string data){
  if(name=="output_user_set_string") ouput_user_set_string(data);
  if(name=="output_table") output_table(data);
  if(name=="input_table")  input_table(data);
  if(name=="mission_normal_backup" & frist_time_get_mission==1){
    frist_time_get_mission=0;
    mission_normalf(data);
  }
  if(name=="mission_normal")  mission_normalf(data);
}
void update_output(){
  //
  if(output_user_status[0]==1) digitalWrite(out1,HIGH);
  else digitalWrite(out1,LOW);
  //
  if(output_user_status[1]==1) digitalWrite(out2,HIGH);
  else digitalWrite(out2,LOW);
  //
  if(output_user_status[2]==1) digitalWrite(out3,HIGH);
  else digitalWrite(out3,LOW);
  //
  if(output_user_status[3]==1) digitalWrite(out4,HIGH);
  else digitalWrite(out4,LOW);
  //
  if(output_user_status[4]==1) digitalWrite(out5,HIGH);
  else digitalWrite(out5,LOW);
  //
  if(output_user_status[5]==1) digitalWrite(out6,HIGH);
  else digitalWrite(out6,LOW);
  //
  if(output_user_status[6]==1) digitalWrite(out7,HIGH);
  else digitalWrite(out7,LOW);
}
void update_input(){
  if(digitalRead(in1)==HIGH) input_user_status[0]=1;
  else input_user_status[0]=0;
  if(digitalRead(in2)==HIGH) input_user_status[1]=1;
  else input_user_status[1]=0;
  if(digitalRead(in3)==HIGH) input_user_status[2]=1;
  else input_user_status[2]=0;
  if(digitalRead(in4)==HIGH) input_user_status[3]=1;
  else input_user_status[3]=0;
  if(digitalRead(in5)==HIGH) input_user_status[4]=1;
  else input_user_status[4]=0;
  if(digitalRead(in6)==HIGH) input_user_status[5]=1;
  else input_user_status[5]=0;
  if(digitalRead(in7)==HIGH) input_user_status[6]=1;
  else input_user_status[6]=0;
  if(digitalRead(in8)==HIGH) input_user_status[7]=1;
  else input_user_status[7]=0;
}
//
void mission_normalf(string msg){
     if((action_mission!=Active_ & msg!="") | msg=="RESET" ){
          if(msg=="RESET") msg="";
          static string data;
          data="";
          for(int i=0;i<msg.length();i++){
              if(msg[i]!=' ' & msg[i]!='\n' & msg!="\t") data+=msg[i];
          }
          //
          cout<<data<<endl;
          my_multiple_mission.data=data;
          my_multiple_mission.delete_free();
          my_multiple_mission.process_data();
          my_multiple_mission.print(0);
      }
}
//
static int first_time=0;
static int httpResponseCode;
static string request_string;
void thread1(void *args)
{
    for (;;) {
      if ( xSemaphoreTake( xSemaphores, ( TickType_t ) 10 ) == pdTRUE ) {
          if(request_post==1){
            request_post=0;
            http.begin(client, serverName);
            http.addHeader("Content-Type", "application/json");
            request_string="{"+data_httppost+input_to_json()+output_to_json()+output_user_set_string_fesp_json()+frist_time_get_mission_json()+"}";
             httpResponseCode= http.POST(request_string.c_str());
             if(httpResponseCode==200){
                http_respone = http.getString();
              }else{
                Serial.print("HTTP Response code: ");
                Serial.println(httpResponseCode);
              }
              // Free resources
              http.end();
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
              }
          }
        xSemaphoreGive( xSemaphores );
        }
        //
        delay(10);
    }
}
void thread2(void *args)
{
    for (;;) {
        if ( xSemaphoreTake( xSemaphores, ( TickType_t ) 10 ) == pdTRUE ) {
                static uint64_t time_num=millis();
                if(millis()-time_num >=1000){
                    time_num=millis();   
                }
           xSemaphoreGive( xSemaphores );
        }
        delay(10);
    }
}
void thread3(void *args)
{
    for (;;) {
        if ( xSemaphoreTake( xSemaphores, ( TickType_t ) 10 ) == pdTRUE ) {
                static uint64_t time_num=millis();
                if(millis()-time_num >=1000){
                    time_num=millis();   
                }
           xSemaphoreGive( xSemaphores );
        }
        delay(10);
    }
}
void http_post(){
  if(request_post==1){
    if ( xSemaphoreTake( xSemaphores, ( TickType_t ) 10 ) == pdTRUE ) {
      request_post=0;
      http.begin(client, serverName);
      http.addHeader("Content-Type", "application/json");
      request_string="{"+data_httppost+input_to_json()+output_to_json()+output_user_set_string_fesp_json()+frist_time_get_mission_json()+"}";
      xSemaphoreGive( xSemaphores );
    }
    httpResponseCode= http.POST(request_string.c_str());
    if ( xSemaphoreTake( xSemaphores, ( TickType_t ) 10 ) == pdTRUE ) {
       if(httpResponseCode==200){
          http_respone = http.getString();
        }else{
          Serial.print("HTTP Response code: ");
          Serial.println(httpResponseCode);
        }
        // Free resources
        http.end();
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
        }
        xSemaphoreGive( xSemaphores );
    }
  }
}
//
Scheduler scheduler;
Task taskHttpPost(1000, TASK_FOREVER, &http_post);
//
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  // network 
  WiFi.begin(ssid, password);
  http.setTimeout(2000);
  input_user_status.resize(num_input_gpio);
  output_user_status.resize(num_output_gpio);
  pinMode(in1, INPUT_PULLUP);
  pinMode(in2, INPUT_PULLUP);
  pinMode(in3, INPUT_PULLUP);
  pinMode(in4, INPUT_PULLUP); 
  pinMode(in5, INPUT_PULLUP);
  pinMode(in6, INPUT_PULLUP);
  pinMode(in7, INPUT_PULLUP);
  pinMode(in8, INPUT_PULLUP); 
  pinMode(out1,OUTPUT);
  pinMode(out2,OUTPUT);
  pinMode(out3,OUTPUT);
  pinMode(out4,OUTPUT);
  pinMode(out5,OUTPUT);
  pinMode(out6,OUTPUT);
  pinMode(out7,OUTPUT);
  digitalWrite (out1,LOW);
  digitalWrite (out2,LOW);
  digitalWrite (out3,LOW);
  digitalWrite (out4,LOW);
  digitalWrite (out5,LOW);
  digitalWrite (out6,LOW);
  digitalWrite (out7,LOW);
  xSemaphores = xSemaphoreCreateMutex();
//  xTaskCreate(thread1, "thread1", 0xFFFF, NULL, 0, NULL);
//  xTaskCreate(thread2, "thread2", 2048, NULL, 1, NULL);
//  xTaskCreate(thread3, "thread3", 2048, NULL, 1, NULL);
  //
  scheduler.addTask(taskHttpPost);
  taskHttpPost.enable();
  scheduler.startNow();
  Serial.println("Set Up done");
}
void loop() {
  // put your main code here, to run repeatedly:
  static int wifi_reconnect=0;
  static int http_post=0;
  static int status_wifi=0;
  static long time_out_wifi=0;
  scheduler.execute();
  if ( xSemaphoreTake( xSemaphores, ( TickType_t ) 10 ) == pdTRUE ) {
          static uint64_t time_num=millis();
          if(millis()-time_num >=1000){
              if(WiFi.status()== WL_CONNECTED){
                status_wifi=1;
                time_out_wifi=0;
                request_post=1;
              }else{
                time_out_wifi=time_out_wifi+1000;
                status_wifi=0;
                http_post=0;
                request_post=0;
              }
              if(time_out_wifi>=10000){
                  wifi_reconnect=1;
                  time_out_wifi=0;
              }
              time_num=millis();
          }
          // misssion here
          static uint64_t time_num2=millis();
          if(millis()-time_num2 >=50){
              //
              update_output();
              update_input();
              //
              input_user_status_2=input_user_status_1;
              input_user_status_1=input_user_status;
              // mission
              static int res;
              if(action_mission!=Active_){
                res=my_multiple_mission.action(Cancel_);
                if(res==Wake_up_) action_mission=Active_;
              }else{
                    res=my_multiple_mission.action(Active_);
                     if(res==Finish_) action_mission=Finish_;
                     action_mission=res;
              }
              time_num2=millis();
          }
      xSemaphoreGive( xSemaphores );
  }
  if(wifi_reconnect){
    wifi_reconnect=0;
    WiFi.reconnect();
  }
  delay(10);   
}
