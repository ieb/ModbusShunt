#pragma once


#include "modbus.h"

extern int16_t readMCUVoltage();
extern int16_t readMCUTemperature();
extern void readSensors();
extern void dumpModbusStatus();

class CommandLine {
    public:
        CommandLine(UartClass * io, HoldingRegisters *holdingRegisters, InputRegisters *inputRegisters) : 
            io{io}, 
            holdingRegisters{holdingRegisters},
            inputRegisters{inputRegisters} {};
        int16_t deviceAddress = 2;
        int16_t serialNumber = 0;
        int16_t shutRangeCurrent = 100;
        int16_t shutRangeVoltage = 75;
        int16_t temp1Offset = 0;   // x100
        int16_t temp1Scale = 1000; // x1000
        int16_t temp2Offset = 0;  // x100
        int16_t temp2Scale = 1000; // x1000
        bool diagnosticsEnabled = false;


        void begin() {
            loadDefaults();
        };

        void showHelp() {
            io->println(F("Shunt Monitor - key presses"));
            io->println(F("  - 'h' or '?' to show this message"));
            io->println(F("  - 's' show status"));
            io->println(F("  - 'd' toggle diagnostics"));
            io->println(F("  - 'r' read sensor"));            
            io->println(F("  - 'i' read input register"));            
            io->println(F("  - 'H' read holding register"));            
            io->println(F("  - 'w' write holding register"));            
            io->println(F("  - 'S' setup"));
            io->println(F("  - 'R' reset"));
            io->println(F("  - 'F' factory reset"));
        };

        bool checkCommand() {
            if (io->available()) {
                char chr = io->read();
                switch ( chr ) {
                    case 'h': showHelp(); return true;
                    case 's': showStatus(); return true;
                    case 'r': readSensors();  return true;
                    case 'i': readInputRegister(); return true;
                    case 'H': readHoldingRegister(); return true;
                    case 'w': writeHoldingRegister(); return true;
                    case 'S': doSetup(); return true;
                    case 'R': doReset(); return true;
                    case 'F': loadDefaults(); return true;
                    case 'd': toggleDiagnostics(); return true;
                }
            }
            return diagnosticsEnabled;
        };
    private:
        UartClass * io;


        void doReset() {
            _PROTECTED_WRITE(RSTCTRL.SWRR, 1);
        }

        void toggleDiagnostics() {
            diagnosticsEnabled = !diagnosticsEnabled;
        };



        void loadDefaults() {
            if ( holdingRegisters->crcValid() ) {
                io->println(F("Eprom Valid"));
                loadSettings();

            } else {
                io->println(F("Eprom Not Valid, saving current settings"));
                saveSettings();
            }
            showStatus();

        };
        char toHex(int n) {
            if ( n <= 9) {
                return '0'+n;
            } else {
                return 'a'-10+n;
            }
        }

        void loadSettings() {            
            deviceAddress = holdingRegisters->getRegister(REG_DEVICE_ADDRESS);
            serialNumber = holdingRegisters->getRegister(REG_SERIAL_NUMBER);
            shutRangeCurrent = holdingRegisters->getRegister(REG_SHUNT_RANGE_CURRENT);
            shutRangeVoltage = holdingRegisters->getRegister(REG_SHUNT_RANGE_VOLTAGE);
            temp1Offset = holdingRegisters->getRegister(REG_TEMPERATURE1_OFFSET);
            temp1Scale = holdingRegisters->getRegister(REG_TEMPERATURE1_SCALE);
            temp2Offset = holdingRegisters->getRegister(REG_TEMPERATURE2_OFFSET);
            temp2Scale = holdingRegisters->getRegister(REG_TEMPERATURE2_SCALE);

        };

        void saveSettings() {
            holdingRegisters->setRegister(REG_DEVICE_ADDRESS, deviceAddress);
            holdingRegisters->setRegister(REG_SERIAL_NUMBER, serialNumber);
            holdingRegisters->setRegister(REG_SHUNT_RANGE_CURRENT, shutRangeCurrent);
            holdingRegisters->setRegister(REG_SHUNT_RANGE_VOLTAGE, shutRangeVoltage);
            holdingRegisters->setRegister(REG_TEMPERATURE1_OFFSET, temp1Offset);
            holdingRegisters->setRegister(REG_TEMPERATURE1_SCALE, temp1Scale);
            holdingRegisters->setRegister(REG_TEMPERATURE2_OFFSET, temp2Offset);
            holdingRegisters->setRegister(REG_TEMPERATURE2_SCALE, temp2Scale);
            holdingRegisters->updateCRC();
        };

        void doSetup() {
            showStatus();
            io->setTimeout(10000);
            io->print(F("Setup - shunt serial number "));io->print(serialNumber);io->print(F(">"));
            bool changed = updateSetting(&serialNumber) || changed;
            io->print(F("Setup - shunt address "));io->print(deviceAddress);io->print(F(">"));
            changed = updateSetting(&deviceAddress) || changed;
            io->print(F("Setup - shunt range current "));io->print(shutRangeCurrent);io->print(F("A >"));
            changed = updateSetting(&shutRangeCurrent) || changed;
            io->print(F("Setup - shunt range voltage "));io->print(shutRangeVoltage);io->print(F("mV >"));
            changed = updateSetting(&shutRangeVoltage) || changed;
            io->print(F("Setup - shunt temp1 Offset "));io->print(temp1Offset);io->print(F(" 0.1C >"));
            changed = updateSetting(&temp1Offset) || changed;
            io->print(F("Setup - shunt temp1 Scale "));io->print(temp1Scale);io->print(F(" x0.001 >"));
            changed = updateSetting(&temp1Scale) || changed;
            io->print(F("Setup - shunt temp2 Offset "));io->print(temp2Offset);io->print(F(" 0.1C >"));
            changed = updateSetting(&temp2Offset) || changed;
            io->print(F("Setup - shunt temp2 Scale "));io->print(temp2Scale);io->print(F(" x0.001 >"));
            changed = updateSetting(&temp2Scale) || changed;

            if ( changed ) {
                saveSettings();
                showStatus();
                io->println(F("Settings changed, device will restart in 5s "));
                delay(5000);
                doReset();
            }
            io->setTimeout(1000);
        };

        void readHoldingRegister() {
            io->setTimeout(10000);
            io->print(F("Read Holding Register >"));
            int16_t reg;
            if ( !readInt(&reg) ) {
                io->setTimeout(1000);
                return;
            }
            io->print(F("\n"));
            io->print(F("holding:"));
            io->print(reg);
            io->print(F(" value:"));
            io->println(holdingRegisters->getRegister(reg));
            io->setTimeout(1000);
        };

        void writeHoldingRegister() {
            io->setTimeout(10000);
            io->print(F("Write Holding Register >"));
            int16_t reg;
            if ( !readInt(&reg) ) {
                io->setTimeout(1000);
                return;
            }
            io->print(F("Value >"));
            int16_t value;
            if ( !readInt(&value) ) {
                io->setTimeout(1000);
                return;
            }
            holdingRegisters->setRegister(reg, value);
            holdingRegisters->updateCRC();
            loadSettings();
            io->println(F("Done"));
            io->setTimeout(1000);
        }

        void readInputRegister() {
            io->setTimeout(10000);
            io->print(F("Input Register >"));
            int16_t reg;
            if ( !readInt(&reg) ) {
                io->setTimeout(1000);
                return;
            }
            io->print(F("\n"));
            io->print(F("input:"));
            io->print(reg);
            io->print(F(" value:"));
            io->println(inputRegisters->getRegister(reg));
            io->setTimeout(1000);
        };




        void showStatus() {
            io->println(F("Status"));
            io->print(F("0 Serial Number  : "));
            io->println(serialNumber);
            io->print(F("1 Device address    : "));
            io->println(deviceAddress);
            io->print(F("2 Shunt Range Current : "));io->print(shutRangeCurrent);io->println(F(" A"));
            io->print(F("3 Shunt Range Voltage : "));io->print(shutRangeVoltage);io->println(F(" mV"));
            io->print(F("4 Temperature1 Offset : "));io->print(temp1Offset);io->println(F(" 0.1C"));
            io->print(F("5 Temperature1 Scale  : "));io->print(temp1Scale);io->println(F(" (x0.001)"));
            io->print(F("6 Temperature2 Offset : "));io->print(temp2Offset);io->println(F(" 0.1C"));
            io->print(F("7 Temperature2 Scale  : "));io->print(temp2Scale);io->println(F(" (x0.001)"));
            io->print((F("MCU V=")));
            io->print(readMCUVoltage());
            io->print((F("mV T=")));
            io->print(readMCUTemperature());
            io->println((F("C")));
            dumpModbusStatus();
        };


        bool readInt(int16_t *v) {
            String line = io->readStringUntil('\r');
            line.trim();
            if ( line.length() > 0 ) {
                *v = line.toInt();
                return true;
            }
            return false;
        };
        bool updateSetting(int16_t * value, int16_t minval =  INT16_MIN, int16_t maxval = INT16_MAX) {
            int16_t newvalue;
            if (readInt(&newvalue)) {
                if ( newvalue > minval && newvalue < maxval ) {
                    // little endian
                    *value = newvalue;
                    io->print(F(" - changed to "));
                    io->println(newvalue);
                    return true;
                } else {
                    io->println(F(" - invalid, no change"));
                }
            } else {
                io->println(F(" - no change"));
            }
            return false;
        };




    private:
        HoldingRegisters *holdingRegisters;
        InputRegisters *inputRegisters;

};


