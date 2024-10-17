#include "SpO2Sensor.h"

SpO2Sensor::SpO2Sensor()
    : low_pass_filter_red(kLowPassCutoff, kSamplingFrequency),
      low_pass_filter_ir(kLowPassCutoff, kSamplingFrequency),
      finger_timestamp(0),
      finger_detected(false) {}

bool SpO2Sensor::begin() {
    if (sensor.begin()) {
        sensor.setup(0x1F, 4, 2, 100, 411, 4096); // Adjust setup parameters as needed
        return true;
    } else {
        return false;
    }
}

float SpO2Sensor::readSpO2() {
    uint32_t irValue = sensor.getIR();
    uint32_t redValue = sensor.getRed();

    if (redValue > kFingerThreshold) {
        if (millis() - finger_timestamp > kFingerCooldownMs) {
            finger_detected = true;
        }
    } else {
        low_pass_filter_red.reset();
        low_pass_filter_ir.reset();
        stat_red.reset();
        stat_ir.reset();

        finger_detected = false;
        finger_timestamp = millis();
    }

    if (finger_detected) {
        float current_value_red = low_pass_filter_red.process(redValue);
        float current_value_ir = low_pass_filter_ir.process(irValue);

        stat_red.process(current_value_red);
        stat_ir.process(current_value_ir);

        float rred = (stat_red.maximum() - stat_red.minimum()) / stat_red.average();
        float rir = (stat_ir.maximum() - stat_ir.minimum()) / stat_ir.average();
        float r = rred / rir;
        float spo2 = kSpO2_A * r * r + kSpO2_B * r + kSpO2_C;

        // Ensure SpO2 is within realistic range
        if (spo2 > 100.0) spo2 = 100.0;
        if (spo2 < 0.0) spo2 = 0.0;

        return spo2;
    }

    return NAN;
}
