#include <ESPAsyncWebSrv.h>
#include <WiFi.h>
#include "index.h"
#include <iostream>
#include <string>
using namespace std;
//const char *ssid = "IB07_916b";
//const char *password = "Mvibot123";
const char *ssid = "MVP_PRO";
const char *password = "MarueiPRO&@1";
AsyncWebServer server(80);
int value;
void setup() {
  Serial.begin(115200);
  //hot_post
  //WiFi.softAP(ssid, password);
  WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  //IPAddress myIP = WiFi.getIP();
  //Serial.print("AP IP address: ");
  //Serial.println(myIP);
  // creat web index
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", index_page);
  });
  //
  Serial.println(index_page.c_str());
  // upload data from esp to web
  server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request){
      String data = String(value);
      request->send(200, "text/plain", data);
  });
  // dowload data from web to esp
  server.onRequestBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
    if (request->url() == "/senddata") {
      string  str=string((char *)data).substr(0, len);
      request->send(200, "text/plain",str.c_str());    
      Serial.printf("[REQUEST]\t%s\r\n",str.c_str());
    }
  });
  server.begin();
}

void loop() {
  //
  value=value+1;
  delay(1000);
}
