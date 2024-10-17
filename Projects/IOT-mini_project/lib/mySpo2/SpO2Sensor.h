#ifndef SPO2SENSOR_H
#define SPO2SENSOR_H

#include <MAX30105.h>
#include <filters.h>

class SpO2Sensor {
public:
    SpO2Sensor();
    bool begin();
    float readSpO2();

private:
    MAX30105 sensor;
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
