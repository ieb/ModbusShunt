# Modbus Battery Sensor.

Mk1 used the ADC on an Attiny3224. Fine for voltage but not really accurate enough for low levels of current on a 75mV 100A shunt. 

This version is built arround a INA219 which is far easier to use. It speaks modbus to communicate with a modbus controller that can control if its on or not.

The controller will be a Attiny or ESP32 driving a eInk display depending. This will only be on when the boat is in service.

A BLE version was created, but in the end I felt it was overcomplicated and would need to deep sleep to avoid draining the battery.


# ToDo

* [x] Basic implementation, with modbus, holding registers and input registers.
* [ ] Implement current monitoring with auto scaling for different ranges using features of the INA219
* [ ] Implement deep sleep and wake on watchdog or external event.
