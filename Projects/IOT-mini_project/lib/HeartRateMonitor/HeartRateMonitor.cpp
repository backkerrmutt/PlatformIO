#include "HeartRateMonitor.h"

HeartRateMonitor::HeartRateMonitor() : lcd(0x27, 16, 2), rateSpot(0), lastBeat(0), beatsPerMinute(0), beatAvg(0),
                                       previousMillis(0), previousValue(0), LED_DISPLAY(2), low_pass_filter_red(kLowPassCutoff, kSamplingFrequency),
                                       low_pass_filter_ir(kLowPassCutoff, kSamplingFrequency),
                                       finger_timestamp(0),
                                       finger_detected(false) {}

void HeartRateMonitor::Init()
{
    lcd.init();
    lcd.backlight();
    Serial.begin(115200);
    pinMode(2, OUTPUT); // LED_SHOW_HEARTBEAT

    // init sensor
    Serial.println("Initializing...");
    if (!particleSensor.begin(Wire, I2C_SPEED_FAST))
    {
        Serial.println("MAX30105 was not found. Please check wiring/power.");
        while (1)
            ;
    }
    Serial.println("Place your index finger on the sensor with steady pressure.");
    particleSensor.setup();
}

// config WiFi
void HeartRateMonitor::WiFiconfig()
{
    bool res;
    // res = wm.autoConnect(); // auto generated AP name from chipid
    res = wm.autoConnect("Heart-Rate_Mini-pro"); // anonymous ap
    // res = wm.autoConnect("AutoConnectAP","password"); // password protected ap

    if (!res)
    {
        Serial.println("Failed to connect");
        ESP.restart();
    }
    else
    {
        // if you get here you have connected to the WiFi
        Serial.println("connected...yeey :)");
    }
    delay(500);
}

void HeartRateMonitor::Update()
{
    float val = calculateBeatAvg();
    if (val == 0)
    {
        if (My_Delay(1000))
        {
            lcd.clear();
            lcd.print(" no Figure ");
        }
    }
    else
    {
        if (HeartRate_Display(val))
        {
            lcd.clear();
            lcd.print(val, 0);
        }
    }
    Serial.println("");
}

bool HeartRateMonitor::HeartRate_Display(float val)
{
    if (val != previousValue)
    {
        previousValue = val;
        return true;
    }
    else
    {
        return false;
    }
}

bool HeartRateMonitor::My_Delay(unsigned long interval)
{

    unsigned long currentMillis = millis();
    if (currentMillis < previousMillis)
    {
        // Reset previousMillis if currentMillis has overflowed
        previousMillis = 0;
    }
    if (currentMillis - previousMillis >= interval)
    {
        previousMillis = currentMillis;
        return true;
    }
    else
    {
        return false;
    }
}

float HeartRateMonitor::calculateBeatAvg()
{
    long irValue = particleSensor.getIR();
    static float beatAvg = 0;

    if (checkForBeat(irValue))
    {
        long delta = millis() - lastBeat;
        lastBeat = millis();

        float beatsPerMinute = 60.0 / (delta / 1000.0);

        if (beatsPerMinute < 255 && beatsPerMinute > 20)
        {
            rates[rateSpot++] = (byte)beatsPerMinute;
            rateSpot %= 4;

            beatAvg = 0;
            for (byte x = 0; x < 4; x++)
            {
                beatAvg += rates[x];
            }
            beatAvg /= 4;
            digitalWrite(LED_DISPLAY, true);
            delay(80);
            digitalWrite(LED_DISPLAY, false);
        }
    }

    if (irValue < 5000)
    {
        return 0;
    }
    else
    {
        return beatAvg;
        // return beatsPerMinute;
    }
    delay(20);
}


float HeartRateMonitor::readSpO2() {
    uint32_t irValue = particleSensor.getIR();
    uint32_t redValue = particleSensor.getRed();

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
