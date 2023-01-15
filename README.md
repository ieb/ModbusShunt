# Modbus Battery Sensor.

Mk1 used the ADC on an Attiny3224. Fine for voltage but not really accurate enough for low levels of current on a 75mV 100A shunt. 

This version is built arround a INA219 which is far easier to use. It speaks modbus to communicate with a modbus controller that can control if its on or not.

The controller will be a Attiny or ESP32 driving a eInk display depending. This will only be on when the boat is in service.

A BLE version was created, but in the end I felt it was overcomplicated and would need to deep sleep to avoid draining the battery.

The device speaks Modbus RTU, 9600 baud, 8N1.


# Holding registers

|| Register || type || units per bit || R/W || description ||
| 0 | int16 | serial number | R | serial number, can be written once only |
| 1 | int16 | device address | R/W | modbus unit or device address |
| 2 | int16 | A | R/W | Shunt full scale current range, default 100A |
| 3 | int16 | mV | R/W | Shunt full scale voltage, default 75mV |
| 4 | int16 | 0.1C | R/W | Temperature probe 1 offset, default 0 |
| 5 | int16 | 0.001x | R/W | Temperature probe 1 scale, default 1000 eg 1x |
| 6 | int16 | 0.1C | R/W | Temperature probe 2 offset, default 0 |
| 7 | int16 | 0.001x | R/W | Temperature probe 2 scale, default 1000 eg 1x |


# Input registers

|| Register || type || units per bit || description ||
|  0   | int16 | mV | Battery voltage |
|  1   | int16 | 0.1 A | Shunt current |
|  2   | int16 | 0.01 C | Temperature probe 1 |  
|  3   | int16 | 0.01 C | Temperature probe 2 |  
|  4   | int16 | bitmap | Lower 8 bits are Serial documented [here](https://github.com/SpenceKonde/megaTinyCore/blob/master/megaavr/extras/Ref_Serial.md#serialgetstatus) bit 8 indicates and error reading voltage, bit 9 indicates and error reading current  |
|  5   | int16 | mV | MCU Supply voltage |
|  6   | int16 | 0.1 C | MCU Temperature |
|  1000  | int16 | counter | modbus frames recieved |
|  1001   | int16 | counter | modbus frames sent |
|  1002   | int16 | counter | modbus errors recieved |
|  1003   | int16 | counter | modbus frames ignored |
|  1004   | int16 | counter | modbus errors sent |
|  1005   | int16 | counter | modbus buffer overflow |



# megaTinyCore 

This code needs megaTiny core 2.6.4 to work as there are some timing bugs that cause it to fail with the default platform IO 2.5.11 version. Upgrading is a pain as it requires installing a custom package. Installs from github fail as the layout of the source repo is wrong and missing files. 

The hacky way is to install manually as its not possible to install from a git repo due to a bug in the pio code that parses versions numbers.

		cd ~/src
		git clone https://github.com/SpenceKonde/megaTinyCore.git
		cd megaTinyCore
		git co -b 2.6.4
		mkdir ~/.platformio/packages/framework-arduino-megaavr-megatinycore@2.6.4
		cd ~/.platformio/packages/framework-arduino-megaavr-megatinycore@2.6.4
		cat << EOF > .piopm
		{"type": "tool", "name": "framework-arduino-megaavr-megatinycore", "version": "2.6.4", "spec": {"owner": "platformio", "id": 11448, "name": "framework-arduino-megaavr-megatinycore", "requirements": null, "uri": null}}
		EOF
		cat << EOF > package.json
		{
		  "name": "framework-arduino-megaavr-megatinycore",
		  "version": "2.6.4",
		  "description": "Arduino Wiring-based Framework for Microchip tinyAVR 0-series and 1-series chips (megaTinyCore)",
		  "keywords": [
		    "framework",
		    "arduino",
		    "microchip",
		    "tinyAVR"
		  ],
		  "repository": {
		    "type": "git",
		    "url": "https://github.com/ieb/megaTinyCore"
		  }
		}
		EOF
		ln -s ~/megaTinyCore/megaavr/* .
		ln -s ~/megaTinyCore/ChangeLog.md .
		ln -s ~/megaTinyCore/Installation.md .
		ln -s ~/megaTinyCore/MakeUPDIProgrammer.md .
		ln -s ~/megaTinyCore/README.md .
		ln -s ~/megaTinyCore/Wiring.md .




# ToDo

* [x] Basic implementation, with modbus, holding registers and input registers.
* [x] Implement test client
* [x] Reimplement modbus frame handling
* [x] Implement sleep mode, wake on frame
* [x] Average current and voltage over time period, eg 30s
* [x] Test frame handling of packets for other units.
* [x] Calibrate NTC sensors.
* [ ] Test write to holding registers.
* [ ] Implement current monitoring with auto scaling for different ranges using features of the INA219
