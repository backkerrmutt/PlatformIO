#ifndef HEART_RATE_MONITOR_H
#define HEART_RATE_MONITOR_H

#include <Arduino.h>
#include <MAX30105.h>
#include <LiquidCrystal_I2C.h>
#include <heartRate.h>
#include <Wire.h>
#include <WiFiManager.h>
#include <filters.h>

class HeartRateMonitor {
public:
    HeartRateMonitor();
    void Init();
    void Update();
    float calculateBeatAvg();
    bool HeartRate_Display(float val);
    bool My_Delay(unsigned long interval);
    void WiFiconfig();
    LiquidCrystal_I2C lcd;
    float readSpO2();
    
    
private:
    // HeartRate
    MAX30105 particleSensor;
    #define RATE_SIZE 4
    byte rates[RATE_SIZE];
    byte rateSpot;
    long lastBeat;
    float beatsPerMinute;
    int beatAvg;
    unsigned long previousMillis;
    int previousValue;
    int LED_DISPLAY;
    WiFiManager wm;

    // SpO2
    const float kSamplingFrequency = 400.0;
    const unsigned long kFingerThreshold = 10000;
    const unsigned int kFingerCooldownMs = 500;
    const float kLowPassCutoff = 5.0;
    const bool kEnableAveraging = false; // Disable averaging
    const int kSampleThreshold = 5;
    const float kSpO2_A = 1.5958422;
    const float kSpO2_B = -34.6596622;
    const float kSpO2_C = 112.6898759;

    LowPassFilter low_pass_filter_red;
    LowPassFilter low_pass_filter_ir;
    MinMaxAvgStatistic stat_red;
    MinMaxAvgStatistic stat_ir;

    long finger_timestamp;
    bool finger_detected;
    
};

#endif
