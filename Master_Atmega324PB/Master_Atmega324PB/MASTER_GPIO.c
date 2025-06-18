/*
 * MASTER_GPIO.c
 * 	   MCU:	ATmega324PB
 * Created: 2023/2024
 *  Author: Andrej Klein, Milan Lacko
 *
 * Typ: Nastavenie GPIO a praca s GPIO
 */

#include "MASTER_GPIO.h"

// Indikator zaciatku behu programu (loopu) pre MASTER:
void Master_start(void){
	for(int x = 0; x < 3; x++){
		LED0_ON;
		LED1_ON;
		_delay_ms(200);
		LED0_OFF;
		LED1_OFF;
		_delay_ms(100);
	}
}

// Nastavenie portov pre LED a tlacidla:
void LD_SW_init(void){
	DDRD = (1<<DDD4)|(1<<DDD5);							// Port PD4 a PD5 - [VYSTUP  - 1] -  LED0 a LED1.
	DDRD &= ~((1<<PORTD6)|(1<<PORTD7));					// Port PD6 a PD7 - [VSTUP   - 0] -  SW0 a SW1.
	PORTD = (1<<PORTD6)|(1<<PORTD7);					// Port PD6 a PD7 - [PULL_UP - 1] -  SW0 a SW1.
}

// Funkcia pre testy s RTC [DS1307]:
 void DS1307_RTC_test(void){
	 if(!SW0){
		 if(LED1)LED1_ON;
		 if(!LED0)LED0_OFF;
		 
// Zmena strany v displeji na page 0:
		 USART_0_Transmit_string("page 0");
		 USART_0_Transmit_char(0xFF);
		 USART_0_Transmit_char(0xFF);
		 USART_0_Transmit_char(0xFF);
		 while(!SW0);									// Cakanie na pustenie tlacidla SW0.
		 
// Citanie dat zo zariadenia DS1307:
//		 uint8_t seconds = DS1307_Read(0x00);			// Cas nieje pouzity v projekte.
		 uint8_t minutes = DS1307_Read(0x01);
		 uint8_t hours	 = DS1307_Read(0x02);
		 uint8_t day_n	 = DS1307_Read(0x03);
		 uint8_t day	 = DS1307_Read(0x04);
		 uint8_t month	 = DS1307_Read(0x05);
		 uint8_t year	 = DS1307_Read(0x06);
		 
// Odoslanie dat do zariadenia NEXTION:
		 USART_0_Transmit_string("year=");
		 USART_0_Transmit_number(BCD_to_Decimal(year));
		 USART_0_Transmit_char(0xFF);
		 USART_0_Transmit_char(0xFF);
		 USART_0_Transmit_char(0xFF);
		 
		 USART_0_Transmit_string("month=");
		 USART_0_Transmit_number(BCD_to_Decimal(month));
		 USART_0_Transmit_char(0xFF);
		 USART_0_Transmit_char(0xFF);
		 USART_0_Transmit_char(0xFF);
		 
		 USART_0_Transmit_string("days=");
		 USART_0_Transmit_number(BCD_to_Decimal(day));
		 USART_0_Transmit_char(0xFF);
		 USART_0_Transmit_char(0xFF);
		 USART_0_Transmit_char(0xFF);
		 
		 USART_0_Transmit_string("hours=");
		 USART_0_Transmit_number(BCD_to_Decimal(hours));
		 USART_0_Transmit_char(0xFF);
		 USART_0_Transmit_char(0xFF);
		 USART_0_Transmit_char(0xFF);
		 
		 USART_0_Transmit_string("minutes=");
		 USART_0_Transmit_number(BCD_to_Decimal(minutes));
		 USART_0_Transmit_char(0xFF);
		 USART_0_Transmit_char(0xFF);
		 USART_0_Transmit_char(0xFF);
		 
		 USART_0_Transmit_string("day_n=");
		 USART_0_Transmit_number(BCD_to_Decimal(day_n));
		 USART_0_Transmit_char(0xFF);
		 USART_0_Transmit_char(0xFF);
		 USART_0_Transmit_char(0xFF);
		 
		 _delay_ms(100);
		 LED1_OFF;
	 }
}

// Funkcia pre testy teploty a vlhkosti [SHT31]:
void SHT31_TempRH_test(void){
	
	if(!SW1){
		if(LED1)LED1_ON;
		if(!LED0)LED0_OFF;	
		
// Zmena strany v displeji na page 0:
		USART_0_Transmit_string("page 0");			
		USART_0_Transmit_char(0xFF);
		USART_0_Transmit_char(0xFF);
		USART_0_Transmit_char(0xFF);
		while(!SW1);									// Cakanie na pustenie tlacidla SW1.

// Odoslanie hodnoty teploty do zariadenia NEXTION:
		float temp = SHT31_ReadTemperature();		
		USART_0_Transmit_string("n2.val=");
		USART_0_Transmit_number((int)temp);
		USART_0_Transmit_char(0xFF);
		USART_0_Transmit_char(0xFF);
		USART_0_Transmit_char(0xFF);

// Odoslanie hodnoty vlhkosti do zariadenia NEXTION:
		float hum = SHT31_ReadHumidity();
		USART_0_Transmit_string("n3.val=");
		USART_0_Transmit_number((int)hum);
		USART_0_Transmit_char(0xFF);
		USART_0_Transmit_char(0xFF);
		USART_0_Transmit_char(0xFF);
					
		_delay_ms(100);
		LED1_OFF;
	}
}