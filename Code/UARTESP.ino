#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL6tL4euKOH"
#define BLYNK_TEMPLATE_NAME "IOT"
#define BLYNK_AUTH_TOKEN "VePZnQx_AfWZIDxgZu_VPY_9SR84KYqX"
#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

char auth[] = "VePZnQx_AfWZIDxgZu_VPY_9SR84KYqX";
char ssid[] = "MANG DAY KTX H1-714";
char pass[] = "23456789";

const byte RX = D6;
const byte TX = D5;
SoftwareSerial mySerial = SoftwareSerial(RX,TX);
String inputString = "";
bool stringComplete = false;

String ChuoiSendEsp = "";

long dienap = 0;
long last = 0;
int da;

void setup()
{
  Serial.begin(9600);
  mySerial.begin(9600);
  last = millis();
  Blynk.begin(auth, ssid, pass);
}

void loop()
{
  Blynk.run();
  Read_UART_JSON();
} 

void Read_UART_JSON()
{
  while (mySerial.available())
  {
     const size_t capacity = JSON_OBJECT_SIZE(2) + 256;
     DynamicJsonDocument doc(capacity);
     DeserializationError error = deserializeJson(doc, mySerial);
     if (error)
     {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
      return;
     }
     else
     {
     
      da = doc["DA"].as<int>();
      Serial.print("Ket qua dien ap thu duoc tu Arduino: ");
      Serial.print(da);
      Serial.println("V");

      char buffer[500];
      size_t n = serializeJson(doc, buffer);
      //client.publish(topicpub.c_str(), buffer, n);
      for(int i = 0;i < 500; i++)
      {
         buffer[i] = 0;
      }
      doc.clear();
      Serial.println("-----------------------------------");
      
      Blynk.virtualWrite(V0,da);
     }
  }
}