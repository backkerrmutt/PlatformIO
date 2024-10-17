#include <SpO2Sensor.h>

SpO2Sensor spo2Sensor;

void setup() {
    Serial.begin(9600);

    if (spo2Sensor.begin()) {
        Serial.println("Sensor initialized");
    } else {
        Serial.println("Sensor not found");
        while (1);
    }
}

void loop() {
    float spo2 = spo2Sensor.readSpO2();
    if (!isnan(spo2)) {
        Serial.print("Time (ms): ");
        Serial.println(millis());
        Serial.print("SpO2 (%): ");
        Serial.println(spo2);
    }

    delay(1000); // Wait for 1 second before the next reading
}
