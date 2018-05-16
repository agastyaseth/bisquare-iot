/*
Send Pronto Hex via an IR LED connected to Arduino Pin D9.
Make sure you do not send a blank (" ") as the last character.
Send the following command over the serial line:
 
Sony12, device = 1, obc = 47
SEND 0000 0067 0000 000d 0060 0018 0030 0018 0030 0018 0030 0018 0030 0018 0018 0018 0030 0018 0018 0018 0030 0018 0018 0018 0018 0018 0018 0018 0018 03de
 
or
 
# RC5, device = 11, obc = 64
SEND 0000 0073 0000 000B 0040 0020 0020 0020 0020 0040 0040 0040 0020 0020 0040 0020 0020 0020 0020 0020 0020 0020 0020 0020 0020 0CC8
 
Based on IR_Player_ProntoCode.c by Stephen Ong
https://github.com/stephenong/Arduino-IR-Remote-Control-Player
This work is licenced under the Creative Commons Attribution-NonCommercial 
3.0 Unported License. To view a copy of this licence, visit 
http://creativecommons.org/licenses/by-nc/3.0/ or send a letter to Creative 
Commons, 171 Second Street, Suite 300, San Francisco, California 94105, USA.
http://creativecommons.org/licenses/by-nc/3.0/
*/

#include <Pronto.h>



Pronto pronto(9);

const uint16_t inputLength = 512;

void setup() {
	Serial.begin(9600);
	while (Serial.available());
	Serial.print("Hello World!\r\n enter \"SEND\" and the Pronto Code.\r\n");
	Serial.print("0000 spaces between the Pronto Hex\r\n");
	Serial.print("no commas and no trailing spaces\r\n");
	Serial.print("This is a working Hex so you can copy and paste it.\r\n");
	//This is one of thoes very ugly Samsung Smart tv power codes. I think it is 42 bit at 37.9khz
	//this is one very long array 78 total cells my sggestion would be to run it through serial
	//or script in loading the Pronto Codes from a file. This would just chew up to much memory. there are almost 200
	//Pronto Codes like this for a Samsung TV My use if for power on only. all the rest of the commands are wifi or even the EXLINK Port
	//that is rs-232
	// Not working yet Serial.println("Example: SEND 0x0 0x6D 0x22 0x3 0xA9 0xA8 0x15 0x3F 0x15 0x3F 0x15 0x3F 0x15 0x15 0x15 0x15 0x15 0x15 0x15 0x15 0x15 0x15 0x15 0x3F 0x15 0x3F 0x15 0x3F 0x15 0x15 0x15 0x15 0x15 0x15 0x15 0x15 0x15 0x15 0x15 0x15 0x15 0x3F 0x15 0x15 0x15 0x15 0x15 0x15 0x15 0x15 0x15 0x15 0x15 0x15 0x15 0x40 0x15 0x15 0x15 0x3F 0x15 0x3F 0x15 0x3F 0x15 0x3F 0x15 0x3F 0x15 0x3F 0x15 0x702 0xA9 0xA8 0x15 0x15 0x15 0xE6E");
	Serial.println("Example: SEND 0000 006d 0022 0003 00a9 00a8 0015 003f 0015 003f 0015 003f 0015 0015 0015 0015 0015 0015 0015 0015 0015 0015 0015 003f 0015 003f 0015 003f 0015 0015 0015 0015 0015 0015 0015 0015 0015 0015 0015 0015 0015 003f 0015 0015 0015 0015 0015 0015 0015 0015 0015 0015 0015 0015 0015 0040 0015 0015 0015 003f 0015 003f 0015 003f 0015 003f 0015 003f 0015 003f 0015 0702 00a9 00a8 0015 0015 0015 0e6e");
}

void loop() {
	if ( Serial.available() > 0 ) {
		static char input[inputLength];
		static uint16_t i;
		char c = Serial.read();
		if ( c != '\r' && c != '\n' && i < inputLength-1)
			input[i++] = c;
		else {
			input[i] = '\0';
			i = 0;
			uint16_t array[80];
			uint16_t j = 0;
			if ( !strncmp(input, "SEND", 4) ) {
				char* p = input+4;
				while ( (p = strchr(p, ' ')) != NULL )
						array[j++] = strtol(p, &p, 16);
				pronto.ir_start(array);
				Serial.print("SENT ");
				for ( uint8_t i = 0; i < j; i++ ) {
					Serial.print ("0x");
					Serial.print (array[i], HEX);
					Serial.print(" ");
				}
				Serial.println();
			}
		}
	}
}
