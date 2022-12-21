#pragma once

#include "Arduino.h"

#include <Wire.h>


// config bit map, register 0, only settings of interest
#define INA219_ADDRESS 0x40


// datasheet page 19 https://www.ti.com/lit/ds/symlink/ina219.pdf
# define IMA219_CONFIG_RUNNING 0b0000111001100111
//                       no reset|||-|---|---|--|
//                        not used|| |   |   |  |
//                          16v____| |   |   |  |
//                01 == 80mV shunt___|   |   |  |
//    1100 = 12bit 16samples 8.5ms_______|   |  |
//    1100 = 12bit 16samples 8.5ms___________|  |
//     Continuous_______________________________|
# define IMA219_CONFIG_STOPPED 0b0000111001100000
//                       no reset|||-|---|---|--|
//                        not used|| |   |   |  |
//                          16v____| |   |   |  |
//                01 == 80mV shunt___|   |   |  |
//    1100 = 12bit 16samples 8.5ms_______|   |  |
//    1100 = 12bit 16samples 8.5ms___________|  |
//     Power down_______________________________|

// registers
#define INA219_REG_CONFIG        0x00
#define INA219_REG_SHUNTVOLTAGE  0x01
#define INA219_REG_BUSVOLTAGE    0x02
#define INA219_REG_POWER         0x03
#define INA219_REG_CURRENT       0x04
#define INA219_REG_CALIBRATION   0x05


#define I2C_OK                   0 // OK
#define I2C_TX_TOO_LONG          1 //: data too long to fit in transmit buffer.
#define I2C_ADDR_NAC             2 //: received NACK on transmit of address.
#define I2C_TX_NAC               3 //: received NACK on transmit of address.
#define I2C_ERROR_OTHER          4 //: other error.
#define I2C_TIMEOUT              5 //: timeout
#define I2C_READ_INCOMPLETE     16 //: incomplete read




class Ina219 {
  public:
    Ina219(UartClass *_debug, uint8_t _address=INA219_ADDRESS): 
      debug{_debug},
      address{_address} {};
    uint8_t begin() {
      Wire.begin();
      return powerDown(false);
    };
    void enableLogging(bool enabled) {
      this->loggingEnabled = enabled;
    }
    /**
     * Get voltage in V
     */
    float getVoltage() {
      int16_t raw = readRegister(INA219_REG_BUSVOLTAGE);
      if ( error == I2C_OK ) {
        int16_t value  = raw >> 3;
        float v =  0.001f * (int16_t)(value * 4);
        if ( loggingEnabled ) {
          debug->print(F("Voltage raw=")); 
          debug->print(raw,BIN); 
          debug->print(F(" v=")); 
          debug->print(value, BIN); 
          debug->print(F(" V=")); 
          debug->println(v,6);
        }
        return v;
      } else {
        debug->print(F("I2C read voltage failed:"));
        debug->println(error);
        return 0;
      }
    }

    /**
     * Get current in A
     */ 
    float getCurrent() {
      int16_t raw = readRegister(INA219_REG_SHUNTVOLTAGE);
      if ( error == I2C_OK ) {
        // +-4096 == +-80mV
        // 1 lsb = 80/4096 = 0.01953125mV
        // 75mV == 100A  1mV == 100/75 == 1.3333333333A
        // 1lsb ==  0.01953125mV == 100*0.01953125/75 ==  0.02604166667A

        // PGA == 2 so the top 3 bits are the sign bit and must be cleared
        // page 21 of the datasheet https://www.ti.com/lit/ds/symlink/ina219.pdf
        int16_t value = raw+1;
//        if ( (value & 0x8000) == 0x8000 ) {
//          value = 0x9fff & value; // keep the sign bit, clear next 2.
//        } 
        float c = 0.02604166667f * ((int16_t) value);
        if ( loggingEnabled ) {
          debug->print(F("Current  raw=")); 
          debug->print(raw); 
          debug->print(F(" v=")); 
          debug->print(value); 
          debug->print(F(" C=")); 
          debug->println(c,6);
        }
        return c;

      } else {
        debug->print(F("I2C Error read current failed:"));
        debug->println(error);
        return 0;
      }
    };

    uint8_t getError() {
      return error;
    };

    uint8_t powerDown(bool down) {
      Wire.beginTransmission(address);
      int16_t config = down?IMA219_CONFIG_STOPPED:IMA219_CONFIG_RUNNING;
      Wire.write(INA219_REG_CONFIG);
      Wire.write(config>>8);
      Wire.write(config&0xff);      
      error =  Wire.endTransmission();

      if ( !down ) {
        int16_t value = readRegister(INA219_REG_CONFIG);
        if ( error == I2C_OK ) {
          if ( value == config ) {
            debug->print(F("Config confirmed as "));
            debug->println(config);
          } else {
            debug->print(F("Config failed requested: "));
            debug->print(config);
            debug->print(F(" set:"));
            debug->println(value);
          }
        } else {
          debug->print(F("Failed to read back confg register "));
          debug->println(error);
        }
      }

      return error;
    };
private:
  UartClass *debug;
  uint8_t error;
  int8_t address;
  bool loggingEnabled = false;

  int16_t readRegister(int8_t r) {
      Wire.beginTransmission(address);
      Wire.write(r);
      error = Wire.endTransmission();
      if ( error != 0) {
        debug->print(F("I2C Failed to select register"));
        debug->println(r, HEX);
        return 0;
      }
      if ( Wire.requestFrom(address, 2) == 2 ) {
        error = I2C_OK;
        int16_t value = Wire.read()<<8 | Wire.read();
#ifdef VERBOSE
        debug->print(F("reg="));
        debug->print(r);
        debug->print(F(" v="));
        debug->println(value);
#endif
        return value;
      } else {
        debug->println(F("I2C Error read current incomplete"));
        error = I2C_READ_INCOMPLETE;
        return 0;
      }
  };

};




