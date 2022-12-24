#!/usr/bin/env python3

import os
import sys
import struct
import logging
from configparser import RawConfigParser
from pymodbus.client.sync import ModbusSerialClient as ModbusClient
from pymodbus.exceptions import ModbusIOException
from pymodbus.constants import Endian
from pymodbus.payload import BinaryPayloadDecoder

logger = logging.getLogger()

#logger.setLevel(logging.DEBUG)
#logging.debug("Debug enabled")

settings = RawConfigParser()
settings.read(os.path.dirname(os.path.realpath(__file__)) + '/batterymon.cfg')

port = settings.get('query', 'port', fallback='/dev/cu.wchusbserial26210')
unit = int(settings.get('battery.main', 'unit', fallback=2))
print("Using ", port, "unit", unit)
print('Setup Serial Connection... ', end='')
client = ModbusClient(method='rtu', port=port, baudrate=9600, stopbits=1, parity='N', bytesize=8, timeout=1)
client.connect()
print('Done!')

def toAscii(registers):
    b = bytearray(len(registers)*2)
    ashex = list()
    i = 0
    for x in registers:
        ashex.append('{:02x}'.format(x))
        struct.pack_into("!H",b,i,x)
        i+=2
    #print(ashex)
    #print(len(registers), b)
    return b.decode('ascii')

def dumpAsHex(registers):
    ashex = list()
    for x in registers:
        ashex.append('{:02x}'.format(x))
    print(ashex)

def packAscii(value):
    registers = list()
    abytes = value.encode('ascii')
    print(abytes)
    i = 0
    v = 0
    for b in abytes:
        v = v*256
        v = v+b
        if i == 1:
            registers.append(v)
            i = 0
            v = 0
        else:
            i += 1
    if i == 1:
        v = v*256
        registers.append(v)
    return registers
        

def dumpAsAscii(registers):
    b = bytearray(len(registers)*2)
    i = 0
    for x in registers:
        struct.pack_into("!H",b,i,x)
        i+=2
    print(len(registers), b)
    print(b.decode('ascii'))

def toInt(row):
    decoder = BinaryPayloadDecoder.fromRegisters(row.registers, Endian.Big, wordorder=Endian.Little)
    return decoder.decode_16bit_int()


# get the serial number
print("Holding Registers")
row = client.read_holding_registers(0, count=1, unit=unit)
print(row)
print("                Serial Number (0000):", toInt(row))
row = client.read_holding_registers(1, count=1, unit=unit)
print("               Device Address (0001):", toInt(row))
row = client.read_holding_registers(2, count=1, unit=unit)
print("          Shunt Current Range (0002):", toInt(row),"A")
row = client.read_holding_registers(3, count=1, unit=unit)
print("          Shunt Voltage Range (0003):", toInt(row), "mV")
row = client.read_holding_registers(4, count=1, unit=unit)
print("   Temperature Probe 1 offset (0004):", 0.1*toInt(row), "C")
row = client.read_holding_registers(5, count=1, unit=unit)
print("    Temperature Probe 1 scale (0005):", 0.001*toInt(row))
row = client.read_holding_registers(6, count=1, unit=unit)
print("   Temperature Probe 2 offset (0006):", 0.1*toInt(row), "C")
row = client.read_holding_registers(7, count=1, unit=unit)
print("    Temperature Probe 2 scale (0007):", 0.001*toInt(row))


print("Input Registers")

row = client.read_input_registers(0, count=1, unit=unit)
print("                 Voltage (0000):", 0.001*toInt(row), "V")
row = client.read_input_registers(1, count=1, unit=unit)
print("                 Current (0001):", 0.01*toInt(row), "A")
row = client.read_input_registers(2, count=1, unit=unit)
print("            Temperature1 (0002):", 0.01*toInt(row), "C")
row = client.read_input_registers(3, count=1, unit=unit)
print("            Temperature2 (0003):", 0.01*toInt(row), "C")
row = client.read_input_registers(4, count=1, unit=unit)
print("            Error bitmap (0004):", '{:016b}'.format(row.registers[0]))
print("                                 ||||||||||||||||")
print("                                 |||||||||||||||^0 serial data written (should be 1)")
print("                                 ||||||||||||||^-1 half duplex enabled")
print("                                 |||||||||||||^--2 serial parity error (clears on read)")
print("                                 ||||||||||||^---3 serial frame error (clears on read)")
print("                                 |||||||||||^----4 serial autobaud enabled")
print("                                 ||||||||||^-----5 serial autoboaud sync enabled, if both 4&5 there was a bad sync")
print("                                 |||||||||^------6 serial overflow ring buffer error - chars dropped, read more frequently, self clears on read")
print("                                 ||||||||^-------7 serial overflow hardware error - chars dropped, irqs disable for too long, self clears on read")
print("                                 |||||||^--------i2c read voltage error")
print("                                 ||||||^---------i2c read current error")
print("                                 |||||^----------unused")
print("                                 ||||^-----------unused")
print("                                 |||^------------unused")
print("                                 ||^-------------unused")
print("                                 |^--------------unused")
print("                                 ^---------------unused")
row = client.read_input_registers(5, count=1, unit=unit)
print("             MCU Voltage (0005):", 0.001*toInt(row), "V")
row = client.read_input_registers(6, count=1, unit=unit)
print("         MCU Temperature (0006):", 0.1*toInt(row), "C")

# Modbus stats
print("Modbus Input Registers")
row = client.read_input_registers(1000, count=1, unit=unit)
print("                Recieved (1000):", toInt(row))
row = client.read_input_registers(1001, count=1, unit=unit)
print("                    Sent (1001):", toInt(row))
row = client.read_input_registers(1002, count=1, unit=unit)
print("         Errors Recieved (1002):", toInt(row))
row = client.read_input_registers(1003, count=1, unit=unit)
print("                 Ignored (1003):", toInt(row))
row = client.read_input_registers(1004, count=1, unit=unit)
print("             Errors Sent (1004):", toInt(row))
row = client.read_input_registers(1005, count=1, unit=unit)
print("         Buffer Overflow (1005):", toInt(row))




