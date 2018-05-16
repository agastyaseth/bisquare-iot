/*
Send Pronto Hex via an IR LED connected to Arduino Pin D9.

Store Pronto Code in uint16_t in either 0x00 or 0000 format commas between each HEX

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

int _pin = 9; //change number to change the pinMode
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
  digitalWrite(_pin,LOW); // Turn output off
//  IR_DDR |= IR_BV; // Set it as output (atmega328 only)
  pinMode(_pin,OUTPUT); // Set it as output
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

// Codes can either be 0x00 or 0000 format
uint16_t up[78] = { 0x0, 0x6D, 0x22, 0x3, 0xA9, 0xA8, 0x15, 0x3F, 0x15, 0x3F, 0x15, 0x3F, 0x15, 0x15, 0x15, 0x15, 0x15, 0x15, 0x15, 0x15, 0x15, 0x15, 0x15, 0x3F, 0x15, 0x3F, 0x15, 0x3F, 0x15, 0x15, 0x15, 0x15, 0x15, 0x15, 0x15, 0x15, 0x15, 0x15, 0x15, 0x15, 0x15, 0x3F, 0x15, 0x15, 0x15, 0x15, 0x15, 0x15, 0x15, 0x15, 0x15, 0x15, 0x15, 0x15, 0x15, 0x40, 0x15, 0x15, 0x15, 0x3F, 0x15, 0x3F, 0x15, 0x3F, 0x15, 0x3F, 0x15, 0x3F, 0x15, 0x3F, 0x15, 0x702, 0xA9, 0xA8, 0x15, 0x15, 0x15, 0xE6E };

void setup() {
	Serial.begin(115200);
	while (Serial.available());
	Serial.print("Hello World!\r\n enter \"SEND\" to send IR\r\n");
}

void loop() {
	if (Serial.available() > 0 ) {
		String inString = Serial.readString();
		if (inString.startsWith("SEND")) {
			int j = 78;
			ir_start(up);
			Serial.print("SENT: ");
			for ( int i = 0; i < j; i++ ) {
				Serial.print ("0x");
				Serial.print (up[i], HEX);
				Serial.print(" ");
			}
			Serial.print("\r\n");
		}
	}
}