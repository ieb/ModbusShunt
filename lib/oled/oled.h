#pragma once

#include <U8g2lib.h>
#include <Wire.h>

#define LINEHEIGHT 20

class Oled {
	public:
		Oled() : u8g2{U8G2_R3} {};
		void begin() {
			  u8g2.begin();
			  u8g2.clear();
			  u8g2.setContrast(32);

		};
		void powerDown(bool powerDown) {
			u8g2.setPowerSave(powerDown);	
		};
		void update() {
			char buffer[10];
			messages++;
		  u8g2.firstPage();
		  do {
		    u8g2.setFont(u8g2_font_helvR14_tr);
		    updateBuffer(buffer, voltage,'V');
		    u8g2.drawStr(0,1*LINEHEIGHT,buffer);
		    if ( current < 0 ) {
			    updateBuffer(buffer, -current,'a');
		    } else {
			    updateBuffer(buffer, current,'A');
		    }
		    u8g2.drawStr(0,2*LINEHEIGHT,buffer);
		    updateBuffer(buffer, temp1,'C');
		    u8g2.drawStr(0,3*LINEHEIGHT,buffer);
		    updateBuffer(buffer, temp2,'C');
		    u8g2.drawStr(0,4*LINEHEIGHT,buffer);
		    itoa(messages, buffer, 10);
		    u8g2.drawStr(0,5*LINEHEIGHT,buffer);
		  } while ( u8g2.nextPage() );
		};
		int16_t voltage = 0;
		int16_t current = 0;
		int16_t temp1 = 0;
		int16_t temp2 = 0;
		uint16_t messages = 0;
	private:
		// 1 page, HW I2C controller.
		U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2;

		void updateBuffer(char * buffer, int16_t v100, char units) {
			if ( v100 >= 10000 ) {
				buffer[0] = 'h';
				buffer[1] = 'i';
				buffer[2] = 'g';
				buffer[3] = 'h';
				buffer[4] = ' ';
			} else if ( v100 < 0 ) {
				buffer[0] = 'l';
				buffer[1] = 'o';
				buffer[2] = 'w';
				buffer[3] = ' ';
				buffer[4] = ' ';
			} else {
				itoa(v100, buffer, 10);
				if ( v100 < 10 ){
					buffer[4] = buffer[0];
					buffer[3] = '0';
					buffer[2] = '.';
					buffer[1] = '0';
					buffer[0] = '0';
				} else if ( v100 < 100 ){
					buffer[4] = buffer[1];
					buffer[3] = buffer[0];
					buffer[2] = '.';
					buffer[1] = '0';
					buffer[0] = '0';
				} else if (v100 < 1000 ) {
					buffer[4] = buffer[2];
					buffer[3] = buffer[1];
					buffer[2] = '.';
					buffer[1] = buffer[0];
					buffer[0] = '0';
				} else {
					buffer[4] = buffer[3];
					buffer[3] = buffer[2];
					buffer[2] = '.';
				}
			}
			buffer[5] = ' ';
			buffer[6] = units;
			buffer[7] = '\0';
		};

};