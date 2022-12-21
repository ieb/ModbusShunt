
#include <Arduino.h>


/**
 * @brief crc for mdbus, polynomial = 0x8005, reverse in, reverse out, init 0xffff;
 * 
 * @param array 
 * @param length 
 * @return uint16_t 
 */
uint16_t modbus_crc16(const uint8_t *array, uint16_t length) {
    uint16_t crc = 0xffff;
    while (length--) {
        if ((length & 0xFF) == 0) yield();  // RTOS
        uint8_t data = *array++;
        data = (((data & 0xAA) >> 1) | ((data & 0x55) << 1));
        data = (((data & 0xCC) >> 2) | ((data & 0x33) << 2));
        data =          ((data >> 4) | (data << 4));
        crc ^= ((uint16_t)data) << 8;
        for (uint8_t i = 8; i; i--) {
        if (crc & (1 << 15)) {
            crc <<= 1;
            crc ^= 0x8005;
        } else {
            crc <<= 1;
        }
        }
    }
    crc = (((crc & 0XAAAA) >> 1) | ((crc & 0X5555) << 1));
    crc = (((crc & 0xCCCC) >> 2) | ((crc & 0X3333) << 2));
    crc = (((crc & 0xF0F0) >> 4) | ((crc & 0X0F0F) << 4));
    //  crc = (( crc >> 8) | (crc << 8));
    //  crc ^= endmask;
    return crc;
};