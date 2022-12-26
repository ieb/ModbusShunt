
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

// MAX485 is on RX0/TX0 Serial, TX0==PB2 pin 7, RX0=PB3 pin 6
// XDIR low to recieve, high to send on PA4 pin 4
// Debug header is on Serial1

#define rs485 Serial
#define debug Serial1


class InputRegisterImpl : public InputRegisters {
	public:
		bool hasRegister(int16_t reg) {
			return ( reg >= 0 && reg <=6 );
		}	
		int16_t getRegister(int16_t reg) {
			int16_t value = 0;
			switch(reg) {
				case 0:
					value = readVoltage();
					break;
				case 1:
					value = readCurrent();
					break;
				case 2:
					value = readTemperature(1);
					break;
				case 3: 
					value = readTemperature(2);
					break;
				case 4: 
					value = (errorBitmap & 0xFF00) | rs485.getStatus();
					break;
				case 5:
					value =  readMCUVoltage();
					break;
				case 6:
					value = 10*readMCUTemperature();
					break;

			}
			return value;
		};
};
// pins
#define TEMP_NTC1_PIN PIN_PA4  // hardware pin 2
#define TEMP_NTC2_PIN PIN_PA5  // hardware pin 3


InputRegisterImpl inputRegisters = InputRegisterImpl();
HoldingRegisters holdingRegisters = HoldingRegisters(NUMBER_REGISTERS);
Modbus modbus = Modbus(&rs485, &debug, &holdingRegisters, &inputRegisters);
Ina219 ina219(&debug);
CommandLine commandLine(&debug, &holdingRegisters, &inputRegisters);
NtcSensor ntcSensor(&debug);

float averageVoltage = 0;
float averageCurrent = 0;

// must be volatile so the compiler doesnt optimnise
bool ina219Ok = true;


/*
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

void dumpModbusStatus() {
	modbus.dumpStatus();
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

	 // setup sleep mode
	 set_sleep_mode(SLEEP_MODE_STANDBY);
	 sleep_enable();

}

int16_t readVoltage() {
	return (int16_t)(1000.0*averageVoltage);;
}
int16_t readCurrent() {
	return  (int16_t)(100*averageCurrent);
}

int16_t readTemperature(int8_t ch) {
	ntcSensor.refresh();
	return ntcSensor.getTemperature(ch);
}


void readShunt() {
	static unsigned long lastRead = 0;
	unsigned long now = millis();
	if ((now - lastRead) > 5000 ) {

		if ( lastRead == 0 ) {
				averageVoltage = ina219.getVoltage();
				averageCurrent = ina219.getCurrent();
		}
		lastRead = now;
		averageVoltage = averageVoltage*0.9 + ina219.getVoltage()*0.1;
		if ( ina219.getError() != I2C_OK) {
			errorBitmap |= 0x0100;
		} else {
			errorBitmap &= 0xFDFF;
		}
		averageCurrent = averageCurrent*0.9 + ina219.getCurrent()*0.1;
		if ( ina219.getError() != I2C_OK) {
			errorBitmap |= 0x0200;
		} else {
			errorBitmap &= 0xFDFF;
		}
	}
}


void powerDownSleep() {
		ina219.powerDown(true);

		//USART0.STATUS &= ~(1<< 4); // enable RXSIF (Rx start frame interupt flag)
		USART0.CTRLB |= 1<<4; // set the SFDEN bit, start of frame detection.
		//USART1.STATUS &= ~(1<< 4); // enable RXSIF (Rx start frame interupt flag)
		USART1.CTRLB |= 1<<4; // set the SFDEN bit, start of frame detection.
		sleep_cpu();
		// Not required, RX clears it. USART0.CTRLB != 1<<4; // clear the SFDEN bit, start of frame detection.
		ina219.powerDown(false);

}



void loop() {
	static unsigned long lastActivity = millis();
  unsigned long now = millis(); 
  readShunt();

  if ( commandLine.checkCommand() ) {
  	lastActivity = now;  	
  }
  modbus.setDiagnostics(commandLine.diagnosticsEnabled);
  if ( modbus.readQuery() ) {
  	lastActivity = now;
  }
  if ( (now - lastActivity) > 60000  ) {
  	debug.println(F("sleeping"));
  	debug.flush();
  	powerDownSleep();
  	debug.println(F("wakeup"));
  	lastActivity = millis();
  }
}


