#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <ArduinoJson.h>

const char *ssid = "Sawaddee";     // Replace with your network SSID
const char *password = "12345678"; // Replace with your network password

// const char* serverName = "https://script.google.com/macros/s/AKfycbz72ZEITNzjNXeu1g9uk8-eKxDzVUG4KpORXw4-toFZ-LonT4QSwiuKbX70CDkxAsVH/exec";  // แทนที่ด้วย URL ของ Web app ของคุณ
const char* serverName = "https://script.google.com/macros/s/AKfycbzNgi8rBw2YHVWN0C24gdBgl0yIXRrOiSUL07lFph005ZOdi9PiR4kQiBXdgH-xRr5i/exec";  // แทนที่ด้วย URL ของ Web app ของคุณ
const char* token = "my_secret_token_db_heart_rate";                                                                                                  // แทนที่ด้วย Token ของคุณ


WiFiClientSecure client;
HTTPClient http;
int count = 0;

// put function declarations here:
void myFunction();
void Send_data();

void setup()
{
  // put your setup code here, to run once:
  myFunction();
  client.setInsecure(); // ข้ามการตรวจสอบ SSL
}

void loop()
{
  // put your main code here, to run repeatedly:
  Send_data();
  delay(3000);
  count++;
}

void Send_data()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    String url = String(serverName) + "?token=" + token;
    http.begin(client, url); // ใช้ WiFiClientSecure

    http.addHeader("Content-Type", "application/json");

    // สร้าง JSON object โดยใช้ ArduinoJson
    DynamicJsonDocument doc(200);
    doc["heartrate"] = analogRead(A0); // แทนที่ด้วยข้อมูลจริงของคุณ
    doc["spo2"] = count;

    String jsonData;
    serializeJson(doc, jsonData);

    Serial.print("JSON Data: ");
    Serial.println(jsonData);

    int httpResponseCode = http.POST(jsonData);

    if (httpResponseCode > 0)
    {
      Serial.print("การส่งสำเร็จ POST: Status : ");
      Serial.println(httpResponseCode);
    }
    else
    {
      Serial.print("เกิดข้อผิดพลาดในการส่ง POST: Status : ");
      Serial.println(httpResponseCode);
    }

    http.end();
  }
  else
  {
    Serial.println("WiFi ถูกตัดการเชื่อมต่อ");
  }
}

// put function definitions here:
void myFunction()
{
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to ");
  Serial.print(ssid);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}