
#pragma once

#include <Arduino.h>

/**
 * @brief ADC readings at temperatures from MIN_NTC_TEMPERATURE to MAX_NTC_TEMPERATURE in sptes of NTC_TABLE_STEP
 * calculated using https://www.skyeinstruments.com/wp-content/uploads/Steinhart-Hart-Eqn-for-10k-Thermistors.pdf
 * https://www.gotronic.fr/pj2-mf52type-1554.pdf MC53-10K B 3950K
ADC 2.5 Bits    4096
Supply  3.3     
Rtop    22      
C   R   V   ADC
-10 56.06   2.3699461952    3882.9198462721
0   32.96   1.9790393013    3242.4579912664
10  20  1.5714285714    2574.6285714286
20  12.51   1.1962619531    1959.9555838887
30  8.048   0.8838658147    1448.1257507987
40  5.312   0.641827768 1051.5706151142
50  3.588   0.4627325309    758.1409785837
60  2.476   0.333829057 546.9455270469
70  1.743   0.2422566651    396.9133201365
80  1.25    0.1774193548    290.6838709677
90  0.911   0.1312164462    214.9850255336
100 0.6744  0.098151219 160.8109572028
110 0.5066  0.0742795447    121.6996059822


ADC reading needs to be referenced to the supply voltage, to account for 
supply voltage changes.

 */
const int16_t ntcTable[] PROGMEM= {
    3883,
    3242,
    2575,
    1960,
    1448,
    1052,
    758,
    547,
    397,
    291,
    215,
    161,
    127
};
#define NTC_TABLE_LENGTH 13
#define MIN_NTC_TEMPERATURE -10.0
#define MAX_NTC_TEMPERATURE 110.0
#define NTC_TABLE_STEP 10.0
#define MAX_SENSORS 4

class NtcSensor {
public:
    NtcSensor(UartClass *debug): 
        debug{debug} {};
    void begin() {
    };
    void enableLogging(bool logEnabled) {
        this->logEnabled = logEnabled;
    };
    void addSensor(uint8_t pin, uint8_t n) {
        if ( n < MAX_SENSORS ) {
            sensors[n] = pin;
        }
    };
    int16_t getTemperature(uint8_t n) {
        if ( n < MAX_SENSORS ) {
            return temperature[n];
        }
        return -320000;
    }

    void refresh() {

        // Nominally the suppy voltage 3.3 which is applied to the 
        // top R. Using VDD as analog reference avoids having to adjust.
        // for a supply voltage offset. We are not interested in the absolute
        // voltage only the ADC reading realive to the supply voltage.
        analogReference(VDD); // 0.0008056640625V per LSB, although we dont need this value.
        delayMicroseconds(100); // wait at least 60us for the reference change to act
        analogSampleDuration(300);
        for (int n = 0; n < MAX_SENSORS; n++) {
            if ( sensors[n] != 0 ) {

                int32_t reading = analogReadEnh(sensors[n], 12, 1);


                float temp = MAX_NTC_TEMPERATURE;
                int16_t cvp = ((int16_t)pgm_read_dword(&ntcTable[0]));
                if ( reading > cvp ) {
                    temp = MIN_NTC_TEMPERATURE;
                } else {
                    for (int i = 1; i < NTC_TABLE_LENGTH; i++) {
                        int16_t cv = ((int16_t)pgm_read_dword(&ntcTable[i]));
                        if ( reading > cv ) {
                            temp = ((cvp-reading)*NTC_TABLE_STEP);
                            temp /= (cvp-cv);
                            temp += ((i-1)*NTC_TABLE_STEP)+MIN_NTC_TEMPERATURE;
                            break;
                        }
                        cvp = cv;
                    }
                }
                // In units of 0.01C
                temperature[n] = 100*temp;
                if ( logEnabled ) {
                    debug->print(F("Sensor "));
                    debug->print(n);
                    debug->print(F(" adc="));
                    debug->print(reading);
                    debug->print(F(" temp="));
                    debug->println(temperature[n]);
                }
            }
        }
    }
private:
    uint8_t pin;
    UartClass *debug;
    bool logEnabled = false;
    uint8_t sensors[MAX_SENSORS] = { 0,0,0,0 }; 
    uint16_t temperature[MAX_SENSORS] = { -33000, -33000, -33000, -33000};
};
