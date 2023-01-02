
#pragma once

#include <Arduino.h>

/**
 * @brief ADC readings at temperatures from MIN_NTC_TEMPERATURE to MAX_NTC_TEMPERATURE in steps of NTC_TABLE_STEP
 * calculated using https://www.skyeinstruments.com/wp-content/uploads/Steinhart-Hart-Eqn-for-10k-Thermistors.pdf
 * https://www.gotronic.fr/pj2-mf52type-1554.pdf MC53-10K B 3950K
 * Vtop doesnt matter if the ADC reference is VTop. To check this try changing Vtop in a spreadsheet.
ADC 4096        
Vtop    5       
Rtop    10                  
C   R   V   ADC
-10 56.06   4.2431123221    3475.95761429
0   32.96   3.8361266294    3142.5549348231
10  20  3.3333333333    2730.6666666667
20  12.51   2.7787649933    2276.3642825411
30  8.048   2.2296099291    1826.4964539007
40  5.312   1.7345872518    1420.973876698
50  3.588   1.3202826023    1081.575507801
60  2.476   0.992305226 812.896441167
70  1.743   0.7421442562    607.9645746402
80  1.25    0.5555555556    455.1111111111
90  0.911   0.4174686097    341.9902850335
100 0.6744  0.3158959754    258.7819830623
110 0.5066  0.2410865551    197.4981059524


ADC reading needs to be referenced to the supply voltage, to account for 
supply voltage changes.

 */
const int16_t ntcTable[] PROGMEM= {
    3476,
    3143,
    2731,
    2276,
    1826,
    1421,
    1082,
    813,
    608,
    455,
    342,
    259,
    197
};
#define NTC_TABLE_LENGTH 14
#define MIN_NTC_TEMPERATURE -10.0
#define MAX_NTC_TEMPERATURE 110.0
#define NTC_TABLE_STEP 10.0
#define MAX_SENSORS 4

//#define DEBUG 1

class NtcSensor {
public:
    NtcSensor(UartClass *debug): 
        debug{debug} {};
    void begin() {
    };
    void addSensor(uint8_t pin, uint8_t n) {
        if ( n < MAX_SENSORS ) {
            sensors[n] = pin;
        }
    };
    int16_t getTemperature(uint8_t n) {
        if ( n >= MAX_SENSORS ) {
            return -32000;
        } else if ( sensors[n] == 255 ) {
            return -32000;            
        } else {

            // Nominally the suppy voltage 5V which is applied to the 
            // top R. Using VDD as analog reference avoids having to adjust.
            // for a supply voltage offset. We are not interested in the absolute
            // voltage only the ADC reading realive to the supply voltage.
            analogReference(VDD); // 0.0008056640625V per LSB, although we dont need this value.
            delayMicroseconds(100); // wait at least 60us for the reference change to act
            analogSampleDuration(300);

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
            int16_t temperature = 100*temp;
#ifdef DEBUG
            debug->print(F("Sensor "));
            debug->print(n);
            debug->print(F(" pin="));
            debug->print(sensors[n]);
            debug->print(F(" adc="));
            debug->print(reading);
            debug->print(F(" temp="));
            debug->println(temperature);

#endif
            return temperature;
        }
    }
private:
    uint8_t pin;
    UartClass *debug;
    bool logEnabled = false;
    uint8_t sensors[MAX_SENSORS] = { 255,255,255,255 }; 
};
