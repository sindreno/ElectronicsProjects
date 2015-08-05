
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#define F_CPU 1000000UL
#include <util/delay.h>




#define ButtonPort	PORTA
#define ButtonDDR	DDRA
#define ButtonMask1	PA0
#define ButtonMask2	PA1
#define ButtonPIN	PINA

#define LEDPort		PORTB
#define LEDDDR		DDRB
#define LEDMask		PB0

volatile int button1_state = 0;
volatile uint8_t button_state = 0;

volatile int button2_state = 0;

volatile int button_flag = 0;

volatile int button1_counter = 0;
volatile int button2_counter = 0;

volatile uint8_t adc_value;

void INITIATE_ADC()
{
	ADMUX=(0<<REFS0)|(0<<REFS1)|(1<<MUX1);		//Set VCC as reference voltage and ADC at PA2
	ADCSRA=(1<<ADEN)|(0<<ADSC);					// Activate ADC. Deactivate reading.
	ADCSRB=(1<<ADLAR);							// Set the ordering of bits in input register
	
}

void ADCRead()
{
	ADCSRA |= (1 << ADSC);				//Read ADC
	
	while (((ADCSRA >> ADSC) & 1)){}	//Wait til ADR reading is done
	adc_value = ADCH;					//Read input register

}


void INITIATE_INTERUPT()
{
	TCCR0B = 1<<CS00 | 1<< CS01;
	TIMSK0 = 1<<TOIE0;
	
	sei();				//Enable Global Interrupt
}

void INITIATE_BUTTON()
{
	ButtonDDR = (0<<ButtonMask1) | (0<<ButtonMask2);
	ButtonPort = (1<<ButtonMask1) | (1<<ButtonMask2);
	
}

void INITIATE_LEDS()
{
	LEDPort = 1<<LEDMask;
	LEDDDR = 1<<LEDMask;
}


ISR(TIM0_OVF_vect)
{
	if (~ButtonPIN & 1<<ButtonMask1)
	{
		button_flag = 1;
	}
	
	if (~ButtonPIN & 1<<ButtonMask2)
	{
		button_flag = 1;
	}

}

void debounce()
{
	unsigned char i;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		for(i = 0; i<5; i++)
		{
			if (~ButtonPIN & 1<<ButtonMask1)
			{
				button1_counter++;
			}
			else
			{
				if (button1_counter >= 4)
				{
					button_state |= (1<<ButtonMask1);
					button_flag = 0;
					button1_counter = 0;
				}
				else
				{
					button1_counter = 0;
				}
			}
			
			if (~ButtonPIN & 1<<ButtonMask2)
			{
				button2_counter++;
			}
			else
			{
				if (button2_counter >= 4)
				{
					//button1_state = 1;
					button_state |= (1<<ButtonMask2);
					button_flag = 0;
					//flag = 0;
					button2_counter = 0;
				}
				else
				{
					button2_counter = 0;
				}
			}
		}
	}
	//return state;
}


void INITIATE_PWM()
{
		DDRB   |= (1 << PB2);						// PWM output on PB2
		TCCR0A = (1 << COM0A1) | (1 << WGM00) | (1<<WGM01) | (1<<WGM01);		// phase correct PWM mode
		OCR0A  = 0b00000000;						// initial PWM pulse width
		
		TCCR0B = (1<<CS01);							// clock source = CLK/8, start PWM
		OCR0A  = 128;
}
int main(void)
{
	INITIATE_INTERUPT();
	INITIATE_LEDS();
	INITIATE_BUTTON();
	INITIATE_ADC();
	INITIATE_PWM();
	

	


	
	
	while(1)
	{
		
		if (button_flag == 1)
		{
			debounce();
			//button1_flag = 0;
			
		}
		
		
		
		if (button_state & (1<<ButtonMask1)) //(button_state == 1)
		
		{
			LEDPort = 0<<LEDMask;				//Turn LED off
			//OCR0A  += 25;
			button_state = 0;
		}
		
		if (button_state & (1<<ButtonMask2)) //(button_state == 1)
		
		{
			LEDPort = 1<<LEDMask;				//Turn LED on
			//OCR0A  -= 25;
			button_state = 0;
		}
	
		ADCRead();
		OCR0A  = adc_value;

		
	}
}