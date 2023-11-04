#include <ros.h>
#include <std_msgs/String.h>
#include <string.h>
//
#include "/home/hitech/catkin_ws/src/module_gpio_v4/module_gpio/src/libary/libary_basic.h"
#include "/home/hitech/catkin_ws/src/module_gpio_v4/module_gpio/src/string_Iv2/string_Iv2.h"
#include "/home/hitech/catkin_ws/src/module_gpio_v4/module_gpio/src/stoi/stoi.h"
#include "/home/hitech/catkin_ws/src/module_gpio_v4/module_gpio/src/stof/stof.h"
#include "/home/hitech/catkin_ws/src/module_gpio_v4/module_gpio/mission/mission_init.h"
//
#define out1    23
#define out2    22
#define out3    21
#define out4    19
#define out5    17
#define out6    18
#define out7    5
#define out8    16

#define in1   32
#define in2   33
#define in3   25
#define in4   26
#define in5   27
#define in6   14
#define in7   12
#define in8   13

//const char* ssid     = "YEU_NGUOI_KO_YEU_MINH";
//const char* password = "123456789";
const char * ssid="MVP_PRO";
const char * password="MarueiPRO&@1";
string name_seri="IB_DEMO";
int num_input_gpio=8,num_output_gpio=8;
multiple_mission my_multiple_mission;
// Set the rosserial socket server IP address
IPAddress server(10,0,70,20);
// Set the rosserial socket server port
const uint16_t serverPort = 11411;
ros::NodeHandle nh;
// pub topics
std_msgs::String IAM_msg,input_user_status_string,output_user_status_string,output_user_set_string;
ros::Publisher IAM_pub("/IAM", &IAM_msg);
ros::Publisher input_user_status_pub("/input_user_status_string", &input_user_status_string);
ros::Publisher output_user_status_pub("/output_user_status_string", &output_user_status_string);
ros::Publisher output_user_set_pub("/output_user_set_string", &output_user_set_string);
// sub topic
void input_user_status_string_zipf( const std_msgs::String& msg){
  static string_Iv2 data1;
  data1.detect(msg.data,"<","",">");
  for(int i=0;i<data1.data1.size();i++){
      //
      if(data1.data1[i]!=""){
          static string_Iv2 data2;
          data2.detect(data1.data1[i],"","|","");
          //
          static vector<int> input;
          input.resize(0);
          input.resize(data2.data1.size()-1);
          for(int j=1;j<data2.data1.size();j++){
              static string_Iv2 data3;
              data3.detect(data2.data1[j],"",":","");
              input[j-1]=stof(data3.data1[1]);
          }
          //
          static int is_have;
          is_have=0;
          for(int j=0;j<my_module.size();j++){
              if(data2.data1[0]==my_module[j].name){
                  is_have=1;
                  my_module[j].input_user=input;
                  break;
              }
          }
          if(is_have==0){
              cout<<"Have_new_moudle_A:"<<data2.data1[0]<<endl;
              my_module.resize(my_module.size()+1);
              //
              my_module[my_module.size()-1].name=data2.data1[0];
              //
              my_module[my_module.size()-1].input_user=input;
           }
      }
  }
}
void output_user_status_string_zipf( const std_msgs::String& msg){
  static string_Iv2 data1;
  data1.detect(msg.data,"<","",">");
  for(int i=0;i<data1.data1.size();i++){
      //
      if(data1.data1[i]!=""){
          //cout<<data1.data1[i]<<endl;
          static string_Iv2 data2;
          data2.detect(data1.data1[i],"","|","");
          //
          static vector<int> output;
          output.resize(0);
          output.resize(data2.data1.size()-1);
          for(int j=1;j<data2.data1.size();j++){
              static string_Iv2 data3;
              data3.detect(data2.data1[j],"",":","");
              output[j-1]=stof(data3.data1[1]);
          }
          //
          static int is_have;
          is_have=0;
          //
          for(int j=0;j<my_module.size();j++){
              if(data2.data1[0] == my_module[j].name){
                  is_have=1;
                  my_module[j].output_user=output;
                  break;
              }
          }
          if(is_have==0){
              cout<<"Have_new_moudle_B:"<<data2.data1[0]<<endl;
              my_module.resize(my_module.size()+1);
              //
              my_module[my_module.size()-1].name=data2.data1[0];
              //
              my_module[my_module.size()-1].output_user=output;
           }
      }
  }
}
void output_user_set_string_zipf( const std_msgs::String& msg){
  static string_Iv2 data1;
  data1.detect(msg.data,"<","",">");
  for(int i=0;i<data1.data1.size();i++){
      //
      static string_Iv2 data2;
      data2.detect(data1.data1[i],"","||","");
      //
      if(data2.data1.size()>=2){
        if(data2.data1[0]==name_seri){
           static string_Iv2 data3;
           data3.detect(data2.data1[1],"(","|",")");
           for(int j=0;j<data3.data1.size();j++){
              if(stoi(data3.data1[j]) < output_user_status.size()){
                  output_user_status[stoi(data3.data1[j])]=stoi(data3.data2[j]);
                  static string result;
                  result="out"+to_string(stoi(data3.data1[j]))+"->"+to_string(stoi(data3.data2[j]));
                  cout<<result<<endl;
              }
           }
        }
      }
  }
}
void mission_normalf(const std_msgs::String& msg){
    action_mission=Cancel_;
    static string data;
    string msg_data=msg.data;
    data="";
    for(int i=0;i<msg_data.length();i++){
        if(msg_data[i]!=' ' && msg_data[i]!='\n' && msg_data[i]!='\t') data+=msg_data[i];
    }
    my_multiple_mission.delete_free();
    my_multiple_mission.data=data;
    my_multiple_mission.process_data();
    my_multiple_mission.print(0);
}
void output_user_setf(const std_msgs::String& msg){
    static string_Iv2 data;
    data.detect(msg.data,"(","|",")");
    for(int i=0;i<data.data1.size();i++){
        if(stoi(data.data1[i]) < output_user_status.size()){
            output_user_status[stoi(data.data1[i])]=stoi(data.data2[i]);
            static string result;
            result="out"+to_string(stoi(data.data1[i]))+"->"+to_string(stoi(data.data2[i]));
            cout<<result<<endl;
        }
    }
}
string start_char="/";
ros::Subscriber<std_msgs::String> sub1("/input_user_status_string_zip", input_user_status_string_zipf);
ros::Subscriber<std_msgs::String> sub2("/output_user_status_string_zip", output_user_status_string_zipf);
ros::Subscriber<std_msgs::String> sub3("/output_user_set_string_zip", output_user_set_string_zipf);
ros::Subscriber<std_msgs::String> sub4("/IB_DEMO/mission_normal",mission_normalf);
ros::Subscriber<std_msgs::String> sub5("/IB_DEMO/output_user_set",output_user_setf);
//
void gpio_init(){
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
  pinMode(out8,OUTPUT);
  digitalWrite (out1,LOW);
  digitalWrite (out2,LOW);
  digitalWrite (out3,LOW);
  digitalWrite (out4,LOW);
  digitalWrite (out5,LOW);
  digitalWrite (out6,LOW);
  digitalWrite (out7,LOW);
  digitalWrite (out8,LOW);
}
void ros_init(){
  // Set the connection to rosserial socket server
  
  nh.getHardware()->setConnection(server, serverPort);
  nh.initNode();
  // Start to be polite
  nh.advertise(IAM_pub);
  nh.advertise(input_user_status_pub);
  nh.advertise(output_user_status_pub);
  nh.advertise(output_user_set_pub);
  // set up for subrice
  nh.subscribe(sub1);
  nh.subscribe(sub2);
  nh.subscribe(sub3);
  nh.subscribe(sub4);
  nh.subscribe(sub5);
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
  //
  if(output_user_status[7]==1) digitalWrite(out8,HIGH);
  else digitalWrite(out8,LOW);
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
void pub_IAM(){
  IAM_msg.data = (name_seri+"|module_gpio").c_str();
  IAM_pub.publish( &IAM_msg );
}
void gpio_status_string(){
  static string data;
  // input
  data=name_seri;
  for(int i=0;i<input_user_status.size();i++){
    data=data+"|"+"in"+to_string(i+1)+":"+to_string(input_user_status[i]);
  }
  input_user_status_string.data=data.c_str();
  input_user_status_pub.publish( &input_user_status_string);
  // output
  data=name_seri;
  for(int i=0;i<output_user_status.size();i++){
    data=data+"|"+"out"+to_string(i+1)+":"+to_string(output_user_status[i]);
  }
  output_user_status_string.data=data.c_str();
  output_user_status_pub.publish( &output_user_status_string);
}
void pub_output_user_set_string(){
  if(gpio_module_set!=""){
      output_user_set_string.data=gpio_module_set.c_str();
      output_user_set_pub.publish(&output_user_set_string);
      gpio_module_set="";
  }
}
void re_connect_wifi(){
  static int n=0;
  if(WiFi.status() != WL_CONNECTED) n++;
  else n=0;
  if(n>=30){
    n=0;
    WiFi.begin(ssid, password);
  }
}
void setup()
{
  // Use ESP32 serial to monitor the process
  Serial.begin(115200);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  // Connect the ESP32 the the wifi AP
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  gpio_init();
  ros_init();
}
void fn_50_ms(){
  static uint64_t time_num=millis();
  static uint64_t time_now=millis();
  //
  time_now=millis();
  if(time_num>time_now){
    time_num=time_now;
  }else{
    if(time_now-time_num >= 50){
      update_input();
      update_output();
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
      time_num=time_now;
    }
  }
}
void fn_200_ms(){
  static uint64_t time_num=millis();
  static uint64_t time_now=millis();
  //
  time_now=millis();
  if(time_num>time_now){
    time_num=time_now;
  }else{
    if(time_now-time_num >= 200){
      pub_output_user_set_string();
      time_num=time_now;
    }
  }
}
void fn_1000_ms(){
  static uint64_t time_num=millis();
  static uint64_t time_now=millis();
  //
  time_now=millis();
  if(time_num>time_now){
    time_num=time_now;
  }else{
    if(time_now-time_num >= 1000){
      pub_IAM();
      gpio_status_string();
      re_connect_wifi();
      //
      time_num=time_now;
    }
  }
}
void loop()
{
    //
    if (!nh.connected()) {
      
    } else {
      Serial.println("Not Connected");
    }
    //
    fn_50_ms();
    fn_200_ms();
    fn_1000_ms();
    nh.spinOnce();
}
