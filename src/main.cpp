
#include <Arduino.h>
#include <Wire.h>

// Holding registers, used by command line, and the modbus holding 
// register class. Must be defined before including.

#define REG_SERIAL_NUMBER 0
#define REG_DEVICE_ADDRESS 1
#define REG_SHUNT_RANGE_CURRENT 2
#define REG_SHUNT_RANGE_VOLTAGE 3
#define REG_TEMPERATURE1_OFFSET 4
#define REG_TEMPERATURE1_SCALE 5
#define REG_TEMPERATURE2_OFFSET 6
#define REG_TEMPERATURE2_SCALE 7
#define NUMBER_REGISTERS 8


#include <avr/sleep.h>
#include "commandline.h"
#include "ina219.h"
#include "ntcsensor.h"
#include "modbus.h"


extern int16_t readVoltage();
extern int16_t readCurrent();
extern int16_t readTemperature(int8_t ch);
extern int16_t readMCUVoltage();
extern int16_t readMCUTemperature();
extern CommandLine commandLine;
int16_t errorBitmap = 0x00;


class InputRegisterImpl : public InputRegisters {
	public:
		bool hasRegister(int16_t reg) {
			return ( reg >= 0 && reg <=3 );
		}	
		int16_t getRegister(int16_t reg) {
			switch(reg) {
				case 0:
					return readVoltage();
				case 1:
					return readCurrent();
				case 2:
					return readTemperature(1);
				case 3: 
					return readTemperature(2);
				case 4: 
					return errorBitmap;
				case 5:
					return readMCUVoltage();
				case 6:
					int16_t t = readMCUTemperature();
					return t*10;
				default:
					return 0;
			}

		};
};
// pins
#define TEMP_NTC1_PIN PIN_PA4  // hardware pin 2
#define TEMP_NTC2_PIN PIN_PA5  // hardware pin 3

// JDY25M is on RX0/TX0 Serial
// Debug header is on Serial1

#define rs485 Serial
#define debug Serial1

InputRegisterImpl inputRegisters = InputRegisterImpl();
HoldingRegisters holdingRegisters = HoldingRegisters(NUMBER_REGISTERS);
Modbus modbus = Modbus(&rs485, &debug, &holdingRegisters, &inputRegisters);
Ina219 ina219(&debug);
CommandLine commandLine(&debug, &holdingRegisters, &inputRegisters);
NtcSensor ntcSensor(&debug);


// must be volatile so the compiler doesnt optimnise
bool ina219Ok = true;

/*
volatile bool shouldWakeUp=false;

// see https://github.com/SpenceKonde/megaTinyCore/blob/e59fc3046dd69f951497506e8f441a6818c28be5/megaavr/extras/Ref_PinInterrupts.md
// Used to detect wake up from sleep.
ISR(PORTA_PORT_vect) {
	byte flags = VPORTA.INTFLAGS;
	// wake up pin is pin PA 6
	if (flags & (1 << 6) ) {
		shouldWakeUp=true;
		VPORTA.INTFLAGS |= (1 << 6); // clear the flag, so it doesnt fire continuously.
	}
}
*/

int16_t readMCUVoltage() {
    analogReference(INTERNAL1V024);
    delayMicroseconds(100);
    int32_t vddmeasure = analogReadEnh(ADC_VDDDIV10, 12); // Take it at 12 bits
    vddmeasure *= 10; // since we measured 1/10th VDD
    int16_t returnval = vddmeasure >> 2; // divide by 4 to get into millivolts.
    if (vddmeasure & 0x02) {
        // if last two digits were 0b11 or 0b10 we should round up
        returnval++;
    }
    return returnval;
}

int16_t readMCUTemperature() {
    int8_t sigrowOffset = SIGROW.TEMPSENSE1;
    uint8_t sigrowGain = SIGROW.TEMPSENSE0;
    analogSampleDuration(128); // must be >= 32µs * f_CLK_ADC per datasheet 30.3.3.7
    analogReference(INTERNAL1V024);
    uint32_t reading = analogRead(ADC_TEMPERATURE);
    reading -= sigrowOffset;
    reading *= sigrowGain;
    reading += 0x80; // Add 1/2 to get correct rounding on division below
    reading >>= 8; // Divide result to get Kelvin
    return reading - 273; // return in C

}

void readSensors() {
		int16_t voltage = (int16_t)(1000.0*ina219.getVoltage());
		// 10mA per bit 320A  0.01*32000
	  int16_t current = (int16_t)(100*ina219.getCurrent());
		// temperature 0.01C per bit, 320C max
		ntcSensor.refresh();
		int16_t temp1 = ntcSensor.getTemperature(0);
		int16_t temp2 = ntcSensor.getTemperature(1);
		uint16_t tons = millis()/1000;

		debug.print(F(" v="));debug.print(voltage);
		debug.print(F(" mV c="));debug.print(current);
		debug.print(F(" 10mA t1="));debug.print(temp1);
		debug.print(F(" C t2="));debug.print(temp2);
		debug.print(F(" C tons="));debug.print(tons);
		debug.println("");

}







void setup() {

  // Open serial communications and wait for port to open:
  debug.begin(115200);

  // try 115200, if that fails drop back to 9600 and set.


  if ( ina219.begin() != I2C_OK) {
  	ina219Ok = false;
	  debug.print(F("Failed to find INA219 chip, error:"));
	  debug.println(ina219.getError());
  }
  commandLine.begin();

  modbus.setDeviceAddress(commandLine.deviceAddress);
  modbus.begin();


  ntcSensor.addSensor(TEMP_NTC1_PIN, 0);
  ntcSensor.addSensor(TEMP_NTC2_PIN, 1);
  ntcSensor.begin();

  commandLine.diagnosticsEnabled = false;


	 if (!ina219Ok) {
	 	 debug.println(F("INA219 not responding"));
	 }
	 debug.println(F("Press h for help menu"));

}

int16_t readVoltage() {
	int16_t voltage =  (int16_t)(1000.0*ina219.getVoltage());
	if ( ina219.getError() != I2C_OK) {
		errorBitmap |= 0x0001;
	}
	return voltage;
}
int16_t readCurrent() {
	int16_t current =  (int16_t)(100*ina219.getCurrent());
	if ( ina219.getError() != I2C_OK) {
		errorBitmap |= 0x0002;
	}
	return current;
}

int16_t readTemperature(int8_t ch) {
	ntcSensor.refresh();
	return ntcSensor.getTemperature(ch);
}






void loop() {
	static unsigned long clKeepAlive = 60000;
	static int state = 0;
	unsigned long now = millis();
  commandLine.checkCommand();
  modbus.setDiagnostics(commandLine.diagnosticsEnabled);
  modbus.readQuery();
}

