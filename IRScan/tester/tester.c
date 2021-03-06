#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include <util/delay.h>
#include "avr_compiler.h"
#include <math.h>

#define STEPS 30 //number of steps in a full sweep of the servo
#define NUM_SERVOS 1 //number of servos in use
#define INCREMENT 1 //change in the duty cycle needed to ellicit a one increment step
#define LENGTH 5 // number of block of data that can be held
#define SERVO_SPEED 50 // this determines the ammount of delay between the movement of the servo
		     // needed because the servo cannot keep up with the instructions given
// AREF pin needs external capacitor for opertation of ADC

struct array_3
{
	int array[LENGTH][STEPS][NUM_SERVOS];
};

int alt(int num)
{
	if (num % 2 == 0)
	{
		return(1);
	}

	else
	{
		return(-1);
	}
}

void setup_servo(int num) // Sets up pwm output for a "num" number of servos(max of 2)
{
	switch(num)
	{
		case 1:
			DDRD |= 0b01000000;     // Set PD6(OC0A) Output
			TCCR0A = 0b10000011;    // Fast PWM mode, Non-inverting mode  
			TCCR0B = 0b00000011;    // clk/256
			OCR0A = 6;            // Set Output Compare Register
			break;
		case 2:
			DDRD |= 0b01100000;     // Set PD6(OC0A) Output Set PD5(OC0B) Output
			TCCR0A = 0b11000011;    // Fast PWM mode, Non-inverting mode  
			TCCR0B = 0b00000010;    // clk/8
			OCR0A = 6;            // Set Output Compare Register
			OCR0B = 6;
			break;
	}
}

void move_servo(int pin, int direction)
{
	if(direction == 1)
	{
		switch(pin)
		{
			case 1:
				OCR0A += INCREMENT;
			case 2:
				OCR0B += INCREMENT;
		}
	}

	else
	{	
		switch(pin)
		{
			case 1:
				OCR0A -= INCREMENT;
			case 2:
				OCR0B -= INCREMENT;
		}
	}
} 



void sweep(struct array_3 * data) // takes a pointer to structure containg 3 dimmensional array, sweeps back and forth

{                         // moving servo first up, then down, and reading for each value
			  // reads a "LENGHT" number of sweeps before returning
	int i, j, k = 0;
	for(i = 0; i < LENGTH; i++)
	{
		for(j = 0; j < STEPS; j++)
		{
			for(k = 0; k < NUM_SERVOS; k++)
			{
				(*data).array[i][j][k] = readadc(k);
				move_servo(k+1, alt(i)); 
				_delay_ms(50);
			}
		}
	}
}
 

int readadc(int channel)
{
	int output;
	ADMUX |= channel; // this selects the register to be read (ADC0 - ADC7)
	ADCSRA |= 0x40;   // start the conversion
	while(!(ADCSRA & 0x10)); // should be interrupt, delay until sample is ready
	output = ADCL + (256 * ADCH); //ADCH hold values above the 8 bit values, therefore must be multiplied by 256
	ADMUX &= ~(0x0F); //clears the selection register
	return output; // returns the raw ADC inteteger
}

void setup_adc(void)
{
	ADMUX = 0b01000000; //use vcc as compare voltage, external cap on AREF pin
	ADCSRA = 0x80; //this enables the ADC 
}
	
void sendchar(char input)
{
	while(!(UCSR0A & 0x20));
	UDR0 = input;
}

void sendstring(char *string)
{
	int i;
	for(i = 0; string[i] != 0; i ++)
	{
		sendchar(string[i]);
	}
}
	

int main(void)
{
	setup_adc();
	setup_servo(1);
	_delay_ms(1000); // delay for 1 seconds
	struct array_3 array;
	sweep(&array);

	//led is turned on, wait for 1 second, turned off
	DDRB |= 0x04;
	if(array.array[0][0][0] > array.array[10][0][0])
	{
		PORTB |=  0x04;
		_delay_ms(1000);
	}
	PORTB = 0x00;
}
