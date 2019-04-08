/*
 * GccApplication8.c
 *
 * Created: 4/5/2019 7:19:59 PM
 * Author : Student
 */ 

#define F_CPU 8000000UL
#define BAUD 9600
#define MYUBRR  (F_CPU/(16UL * BAUD)) - 1

//ADC channels for more information see Atmega328p datasheet.
#define ADC0 0x00
#define ADC1 0x01
#define AVCC 5000 //5V = 5000mV
#define PRECISION 1024
#define T (AVCC/PRECISION)
//##################################
#include <avr/io.h>
#include <util/delay.h>


void initUSART(void);
void usart_Transmit(uint8_t data);
void usart_Message(uint8_t* msg);
uint8_t  usart_Receive(void);

void initADC(void);
uint16_t readADC(uint8_t ADCchannel);

//########################################
void initUSART(void){
	/*Set baud rate */
	UBRR0H = (uint8_t)(MYUBRR>>8);
	UBRR0L = (uint8_t)MYUBRR;
	//Enable receiver and transmitter
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);
	/* Set frame format: 8N1 */
	UCSR0C|=(1<<UCSZ00)|(1<<UCSZ01);   // 8bit data format
	//  UCSR0C = (3<<UCSZ00);
}

void usart_Transmit(uint8_t data){
	
	while (!( UCSR0A & (1<<UDRE0))) ;          // wait while register is free
	UDR0 = data;                             // load data in the register
}


uint8_t  usart_Receive(void){
	
	while(!(UCSR0A & (1<<RXC0)));
	return UDR0;
	
}

void usart_Message(uint8_t* msg){
	
	while(*msg){
		usart_Transmit(*msg);
		msg ++;
		
	}
	
	
}
//###########################################################################
void initADC(void)
{
	// AV CC with external capacitor at AREF pin
	ADMUX |= (1<<REFS0);
	//set prescaller to 128 and enable ADC
	ADCSRA |= (1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0)|(1<<ADEN);
}

uint16_t readADC(uint8_t ADCchannel){
	ADMUX = (ADMUX & 0xF0) | (ADCchannel & 0x0F);
	ADCSRA |= (1 << ADSC);//start conversation
	//Wait until ADC converstain complete. ADSC will be ZERO after completion.
	while(ADCSRA & (1<<ADSC));
	return ADC;
	
}



void main (void){
	uint16_t voltage;
	uint8_t val1, val2,val3,temperature;
	//DDRD |= (1 << PD1);  //TX as an output (PD1)
	//DDRD &= ~(1 << PD0);
	//DDRB |= (1 << PB0);
	initUSART();
	initADC();
	_delay_ms(500);
	//usart_Message("LM35 Temperature Sensor\r\n");
	//  usart_Init();
	
	usart_Message("AT+CIOBAUD=9600\r\n"); // sets the baud for esp8266 to 9600
	_delay_ms(1000);
	usart_Message("AT+CWMODE=1\r\n");
	_delay_ms(1000);
	usart_Message("AT+CWJAP=\"tom_sucks\",\"dickdickdick\"\r\n"); // wifi setup
	_delay_ms(10000);
	
	while(1){
		voltage = readADC(ADC0);
		voltage = ((T*voltage)/10) + 2;
		
		if (voltage >= 100){
			val1 = voltage / 100;
			val2 = voltage % 100;
			val3 = val2 % 10;
			val2 = val2 / 10;
		}
		else{
			val1 = 0;
			val2 = voltage / 10;
			val3 = voltage % 10;
		}
		//Decimal 48=0 Decimal 49=1  Decimal 50 =2 For more information see ASCII table.
		
		
		usart_Message("AT+CIPSTART=\"TCP\",\"184.106.153.149\",80\r\n"); // connect to thingspeak using TCP
		_delay_ms(1000);
		
		// send data to thingspeak
		usart_Message("AT+CIPSEND=49\r\n"); // send the following 49 characters
		_delay_ms(500);
		usart_Message("GET /update?api_key=F4Z9UX1HHBSOMKS7&field1="); // protocol to send data// write key
		usart_Transmit(val1 + 48);
		usart_Transmit(val2 + 48);
		usart_Transmit(val3 + 48);
		usart_Message("\r\n");
		_delay_ms(1000);
		usart_Message("AT+CIPSEND=49\r\n");
		_delay_ms(500);
		usart_Message("GET /update?api_key=F4Z9UX1HHBSOMKS7&field1="); ///write keys
		usart_Transmit(val1 + 48);
		usart_Transmit(val2 + 48);
		usart_Transmit(val3 + 48);
		usart_Message("\r\n");
		_delay_ms(1000);
		
		for(int i = 0; i < 15; i++){
			_delay_ms(1000);
		}
	}
	
}
