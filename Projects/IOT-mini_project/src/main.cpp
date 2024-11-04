// blynk
#define BLYNK_TEMPLATE_ID "TMPL6pw8DvVVt"
#define BLYNK_TEMPLATE_NAME "test"
#define BLYNK_AUTH_TOKEN "0r1fs5N9z-ubMvPkczuytArusIhgDVet"

#include <Arduino.h>
#include <HeartRateMonitor.h>
#include <BlynkSimpleEsp8266.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <ArduinoJson.h>
#include <TimeLib.h>
#include <Timezone.h>


HeartRateMonitor hrm;                                                                                                                        // สร้าง object จาก library HeartRateMonitor
const char *serverName = "https://script.google.com/macros/s/AKfycby7A8DrPq3h8AEoJXmGsOCi_Ul304SQdzu18aiWK7k7eak2LQPb2fN9dbFAqE6Llctu/exec"; // แทนที่ด้วย URL ของ Web app ของคุณ
const char *token = "my_secret_token_db_heart_rate";

WiFiClientSecure client;
HTTPClient http;

const int capacity = 18; // เก็บข้อมูล 60 ครั้ง (1 นาที) 20
unsigned long previousMillis = 0;
const long interval = 500;    // Interval to send data (e.g., every 10 seconds)
StaticJsonDocument<4000> doc; // ขนาดใหญ่ขึ้นเพื่อเก็บข้อมูลทั้งหมด
JsonArray dataArray = doc.createNestedArray("data");
int dataindex = 0;

// กำหนดกฎการเปลี่ยนเวลา (ประเทศไทยไม่มีการเปลี่ยนเวลา)
TimeChangeRule mySTD = {"ICT", Last, Sun, Mar, 2, 420};  // ICT (Indochina Time) UTC +7
Timezone myTZ(mySTD, mySTD);

float heartrate; // heartrate value
float spo2;      // spo2 value

// Machine_Stats Values
const unsigned int IDLE = 0;       // state รอก่อนเลือกโหมด
const unsigned int CHILD = 1;      // state เด็ก
const unsigned int MIDDLE_AGE = 2; // state วัยกลางคน
unsigned int state;                // ค่าเปลี่ยน state

// set switch
const int btn_red = 14;    // D5
const int btn_yellow = 12; // D6
const int btn_white = 13;  // D7

// interrupt status
int flag_red = false;
int flag_yellow = false;
int flag_white = false;

// Global Declaration !!!
void calculateHeartRate(String name, int Lower_quartile, int Upper_quartile);
void handleSwitchYellowInterrupt();
void handleSwitchWhiteInterrupt();
void handleSwitchRedInterrupt();
String getFormattedTime();
time_t getTimeFunction();
void backtoIDEL();
void Send_data();

// Configure the minimum maximum of heart rate...
// CHILD
const int ch_Lower_quartile = 75;
const int ch_Upper_quartile = 110;
// MIDDLE_AGE
const int md_Lower_quartile = 60;
const int md_Upper_quartile = 95;

void setup()
{
  state = IDLE; // config start state to IDLE
  pinMode(btn_yellow, INPUT_PULLDOWN_16);
  pinMode(btn_white, INPUT_PULLDOWN_16);
  pinMode(btn_red, INPUT_PULLDOWN_16);
  hrm.Init(); // เรียกfunction init จาก Library ( ฟังก์ชั้นนี้จะทำการ Initializing sensor และ init_LCD ,init_Serail)
  // interrupt
  attachInterrupt(digitalPinToInterrupt(btn_yellow), handleSwitchYellowInterrupt, RISING);
  attachInterrupt(digitalPinToInterrupt(btn_white), handleSwitchWhiteInterrupt, RISING);
  attachInterrupt(digitalPinToInterrupt(btn_red), handleSwitchRedInterrupt, RISING);
  hrm.WiFiconfig();

  Blynk.begin(BLYNK_AUTH_TOKEN, WiFi.SSID().c_str(), WiFi.psk().c_str()); // เริ่ม Blynk
  client.setInsecure();                                                   // ข้ามการตรวจสอบ SSL
}
void loop()
{
  Blynk.run(); // Run Blynk

  // IDLE start -----------------------------------------------------------------------------------------
  if (state == IDLE)
  {
    if (hrm.My_Delay(1000))
    {
      hrm.lcd.clear();
      hrm.lcd.setCursor(0, 0);
      hrm.lcd.print("Select a mode...");
      hrm.lcd.setCursor(0, 1);
      hrm.lcd.print("Child || Adult");
    }
    // อ่านค่าจาก button ถ้าเลือก btn_yellow ไปโหมดเด็ก แต้ถ้าเลือก btn_white ไปโหมดวัยกลางคน *** pull-down (กดเป็น 1)
    // btn_red return to IDLE
    if (flag_yellow == true) // yellow_btn
    {
      flag_yellow = !flag_yellow;
      delay(70);
      if (digitalRead(btn_yellow == 1))
      {
        Serial.println("yellow do");
        state = MIDDLE_AGE;
      }
    }
    else if (flag_white == true) // white_btn
    {
      flag_white = !flag_white;
      delay(70);
      if (digitalRead(btn_white == 1))
      {
        Serial.println("white do");
        state = CHILD;
      }
    }
  }
  // IDEL end -----------------------------------------------------------------------------------------

  // CHILDEN start -----------------------------------------------------------------------------------------
  else if (state == CHILD)
  {
    String NameOfState = "MODE CHILD";        
    calculateHeartRate(NameOfState, ch_Lower_quartile, ch_Upper_quartile);
    if (flag_red == true) // red_btn
    {
      flag_red = !flag_red;
      delay(50);
      if (digitalRead(btn_red == 1))
      {
        Serial.println("Red do");
        backtoIDEL();
      }
    }
  }
  // CHILDEN end -----------------------------------------------------------------------------------------

  // MIDDLE_AGE start -----------------------------------------------------------------------------------------
  else if (state == MIDDLE_AGE)
  {
    String NameOfState = "Mode Adult";
    calculateHeartRate(NameOfState, md_Lower_quartile, md_Upper_quartile);
    if (flag_red == true) // red_btn
    {
      flag_red = !flag_red;
      delay(70);
      if (digitalRead(btn_red == 1))
      {
        Serial.println("Red do");
        backtoIDEL();
      }
    }
  }
  // MIDDLE_AGE end -----------------------------------------------------------------------------------------
}

// คำนวณและแสดงสถานะออก lCD ----------------------------------------------------------------------------
void calculateHeartRate(String name, int Lower_quartile, int Upper_quartile)
{
  heartrate = hrm.calculateBeatAvg() ;
  Serial.println(heartrate);
  int IntHeartrate = int(heartrate);
  if (heartrate == 0) // ไม่ได้วางนิ้ว
  {
    if (hrm.My_Delay(500))
    {
      hrm.lcd.clear();
      hrm.lcd.setCursor(0, 0);
      hrm.lcd.print(name);
      hrm.lcd.setCursor(0, 1);
      hrm.lcd.print("No Figure !!!");
    }
  }
  else // วางนิ้ว
  {
    if (hrm.My_Delay(500)) // เปลี่ยนค่าทุกๆ 500 milli
    {
      hrm.lcd.clear();
      if (IntHeartrate < Lower_quartile) // อัตราการเต้นของหัวใจต่ำกว่าปกติ
      {
        hrm.lcd.setCursor(0, 0);
        hrm.lcd.print("BPM ");
        hrm.lcd.setCursor(5, 0);
        hrm.lcd.print(heartrate, 0);
        hrm.lcd.setCursor(8, 0);
        hrm.lcd.print("Spo2  ");
        hrm.lcd.setCursor(13, 0);
        hrm.lcd.print(spo2, 0);
        hrm.lcd.setCursor(0, 1);
        hrm.lcd.print("BPM is slower !!!");
        Serial.println("The heart beats slower than usual.");
        if (heartrate == 0)
        {
          delay(1000);
        }
      }
      else if (IntHeartrate > Upper_quartile) // อัตราการเต้นของหัวใจสูงกว่าปกติ
      {
        hrm.lcd.setCursor(0, 0);
        hrm.lcd.print("BPM ");
        hrm.lcd.setCursor(5, 0);
        hrm.lcd.print(heartrate, 0);
        hrm.lcd.setCursor(8, 0);
        hrm.lcd.print("Spo2  ");
        hrm.lcd.setCursor(13, 0);
        hrm.lcd.print(spo2, 0);
        hrm.lcd.setCursor(0, 1);
        hrm.lcd.print("BPM is faster !!!");
        Serial.println("My heart beats faster than usual.");
        if (heartrate == 0)
        {
          delay(1000);
        }
      }
      else if ((IntHeartrate >= Lower_quartile) && (IntHeartrate <= Upper_quartile)) // อัตราการเต้นของหัวใจอยู่ในช่วงปกติ
      {
        hrm.lcd.setCursor(0, 0);
        hrm.lcd.print("BPM ");
        hrm.lcd.setCursor(5, 0);
        hrm.lcd.print(heartrate, 0);
        hrm.lcd.setCursor(8, 0);
        hrm.lcd.print("Spo2  ");
        hrm.lcd.setCursor(13, 0);
        hrm.lcd.print(spo2, 0);
        hrm.lcd.setCursor(0, 1);
        hrm.lcd.print("BPM is normal ..."); 
        Serial.println("Heart rate is within normal range.");
        if (heartrate == 0)
        {
          delay(1000);
        }
      }

      spo2 = hrm.readSpO2();             // read data spo2
      Blynk.virtualWrite(V1, heartrate); // send data to blynk : heartrate
      Blynk.virtualWrite(V2, spo2);      // send data to blynk : spo2
      // send data to database
      unsigned long currentMillis = millis();
      if (currentMillis - previousMillis >= interval)
      {
        previousMillis = currentMillis;
        JsonObject data = dataArray.createNestedObject();
        data["datetime"] = getFormattedTime();
        data["heartrate"] = heartrate;
        data["spo2"] = spo2;
        if (dataindex >= capacity)
        {
          // ส่งข้อมูลทั้งหมด
          Send_data();

          // ล้างข้อมูลใน array และรีเซ็ตตัวนับ
          dataArray.clear();
          dataindex = 0;
        }
        dataindex++;
      }
    }
  }
}
// end --------------------------------------------------------------------------------------------------------

// send data to database
void Send_data()
{
  String url = String(serverName) + "?token=" + token;
  http.begin(client, url); // Use WiFiClientSecure

  http.addHeader("Content-Type", "application/json");
  String jsonData;
  serializeJson(doc, jsonData);

  Serial.print("JSON Data: ");
  Serial.println(jsonData);

  int httpResponseCode = http.POST(jsonData);

  if (httpResponseCode > 0)
  {
    Serial.print("POST Success: Status : ");
    Serial.println(httpResponseCode);
  }
  else
  {
    Serial.print("POST Error: Status : ");
    Serial.println(httpResponseCode);
  }
  http.end();
}
// backtoIDEL_function start ----------------------------------------------------------

void backtoIDEL()
{
  hrm.lcd.clear();
  hrm.lcd.setCursor(0, 0);
  hrm.lcd.print("GoTO Homepage...");
  hrm.lcd.setCursor(0, 1);
  hrm.lcd.print("please wait ......");
  state = IDLE;
  delay(1500);
}
// backtoIDEL_function end ----------------------------------------------------------

// date time
String getFormattedTime() {
  time_t utc = now();  // เวลาปัจจุบันในรูปแบบ UTC
  time_t local = myTZ.toLocal(utc);  // แปลงเวลาเป็นเวลาท้องถิ่น

  char buffer[20];
  sprintf(buffer, "%02d/%02d/%04d %02d:%02d:%02d", month(local), day(local), year(local), hour(local), minute(local), second(local));
  return String(buffer);
}
time_t getTimeFunction() {
  return now();  // ฟังก์ชันตัวอย่างสำหรับการซิงค์เวลา
}


// interrupt
void ICACHE_RAM_ATTR handleSwitchYellowInterrupt()
{
  flag_yellow = !flag_yellow;
}
void ICACHE_RAM_ATTR handleSwitchWhiteInterrupt()
{
  flag_white = !flag_white;
}
void ICACHE_RAM_ATTR handleSwitchRedInterrupt()
{
  flag_red = !flag_red;
}
