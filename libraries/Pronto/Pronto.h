/*
  Pronto.h - Library for sending Pronto hex infrared codes.
*/

#ifndef Pronto_h
#define Pronto_h

#include "Arduino.h"

class Pronto {
	public:
		Pronto(int pin);
		void ir_start(uint16_t *code);
		void handleInterrupt(); //Interrupt Timer Overflow Function
	private:
		int _pin;
		void ir_on();
		void ir_off();
		void ir_toggle();
		void ir_stop();
};

#endif