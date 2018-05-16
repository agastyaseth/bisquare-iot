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

#include <avr/interrupt.h>
#include <stdio.h>
#include <avr/pgmspace.h>
#include <stdint.h>
#include <avr/io.h>
#define IR_PORT PORTB
// #define IR_PIN PINB
// #define IR_DDR DDRB
// #define IR_BV _BV(1)
#define IR_OCR OCR1A
#define IR_TCCRnA TCCR1A
#define IR_TCCRnB TCCR1B
#define IR_TCNTn TCNT1
#define IR_TIFRn TIFR1
#define IR_TIMSKn TIMSK1
#define IR_TOIEn TOIE1
#define IR_ICRn ICR1
#define IR_OCRn OCR1A
#define IR_COMn0 COM1A0
#define IR_COMn1 COM1A1
#define PRONTO_IR_SOURCE 0 // Pronto code byte 0
#define PRONTO_FREQ_CODE 1 // Pronto code byte 1
#define PRONTO_SEQUENCE1_LENGTH 2 // Pronto code byte 2
#define PRONTO_SEQUENCE2_LENGTH 3 // Pronto code byte 3
#define PRONTO_CODE_START 4 // Pronto code byte 4

static const uint16_t *ir_code = NULL;
static uint16_t ir_cycle_count = 0;
static uint32_t ir_total_cycle_count = 0;
static uint8_t ir_seq_index = 0;
static uint8_t ir_led_state = 0;

void ir_on()
{
  IR_TCCRnA |= (1<<IR_COMn1) + (1<<IR_COMn0);
  ir_led_state = 1;
}

void ir_off()
{
  IR_TCCRnA &= ((~(1<<IR_COMn1)) & (~(1<<IR_COMn0)) );
  ir_led_state = 0;
}

void ir_toggle()
{
  if (ir_led_state)
    ir_off();
  else
    ir_on();
}

void ir_start(uint16_t *code)
{
  ir_code = code;
//  IR_PORT &= ~IR_BV; // Turn output off (atmega328 only)
  digitalWrite(9,LOW); // Turn output off
//  IR_DDR |= IR_BV; // Set it as output (atmega328 only)
  pinMode(9,OUTPUT); // Set it as output
  IR_TCCRnA = 0x00; // Reset the pwm
  IR_TCCRnB = 0x00;
  //printf_P(PSTR("FREQ CODE: %hd\r\n"), code[PRONTO_FREQ_CODE]);
  uint16_t top = ( (F_CPU/1000000.0) * code[PRONTO_FREQ_CODE] * 0.241246 ) - 1;
  //printf_P(PSTR("top: %hu\n\r"), top);
  IR_ICRn = top;
  IR_OCRn = top >> 1;
  IR_TCCRnA = (1<<WGM11);
  IR_TCCRnB = (1<<WGM13) | (1<<WGM12);
  IR_TCNTn = 0x0000;
  IR_TIFRn = 0x00;
  IR_TIMSKn = 1 << IR_TOIEn;
  ir_seq_index = PRONTO_CODE_START;
  ir_cycle_count = 0;
  ir_on();
  IR_TCCRnB |= (1<<CS10);
}

#define TOTAL_CYCLES 80000 // Turns off after this number of
// cycles. About 2 seconds
// FIXME: Turn off after having sent
ISR(TIMER1_OVF_vect) {
  uint16_t sequenceIndexEnd;
  uint16_t repeatSequenceIndexStart;
  ir_total_cycle_count++;
  ir_cycle_count++;

  if (ir_cycle_count== ir_code[ir_seq_index]) {
    ir_toggle();
    ir_cycle_count = 0;
    ir_seq_index++;
    sequenceIndexEnd = PRONTO_CODE_START +
      (ir_code[PRONTO_SEQUENCE1_LENGTH]<<1) +
      (ir_code[PRONTO_SEQUENCE2_LENGTH]<<1);

    repeatSequenceIndexStart = PRONTO_CODE_START +
      (ir_code[PRONTO_SEQUENCE1_LENGTH]<<1);

    if (ir_seq_index >= sequenceIndexEnd ) {
      ir_seq_index = repeatSequenceIndexStart;

      if(ir_total_cycle_count>TOTAL_CYCLES) {
        ir_off();
        TCCR1B &= ~(1<<CS10);
      }
    }
  }
}

void ir_stop()
{
  IR_TCCRnA = 0x00; // Reset the pwm
  IR_TCCRnB = 0x00;
}

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
				ir_start(array);
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
