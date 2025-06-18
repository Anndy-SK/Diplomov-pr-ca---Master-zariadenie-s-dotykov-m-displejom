/*
 * MASTER_USART.c
 * 	   MCU:	ATmega324PB
 * Created: 2023/2024
 *  Author: Andrej Klein, Milan Lacko
 *
 * Typ: Nastavenie USART_0 (RXD0[PORTD0], TXD0[PORTD1]) zbernice v MCU a komunikacie s NEXTION display-om.
 *		Nastavenie USART_1 (RXD1[PORTD2], TXD1[PORTD3]) zbernice v MCU a komunikacie so SLAVE-om.
 */ 

#include "MASTER_USART.h"

// Inicializacia USART_0 komunikacie:
void USART_0_Init(void){
// Nastavenie BAUD RATE:
	UBRR0H = (unsigned char)((F_CPU/16/BAUD)-1) >> 8;
	UBRR0L = (unsigned char)((F_CPU/16/BAUD)-1);
	
// Enable RX receiver, TX transmitter and interrupt from RXC:
	UCSR0B |= (1<<RXEN);									// Povolenie prijimania.
	UCSR0B |= (1<<TXEN);									// Povolenie vysielania.
	UCSR0B |= (1<<RXCIE);									// Povolenie prerusenia od prijimu.

	UCSR0C = (1<<USBS)|(1<<UCSZ1)|(1<<UCSZ0);				// Nastavenie frame formatu: 2stop bit, 8data bit
}

// Funkcia na odoslanie textoveho znaku (5 az 8 datoveho bitu) cez USART_0:
void USART_0_Transmit_char(unsigned char data){
	while (!(UCSR0A & (1<<UDRE)))							// Wait for empty transmit buffer.
		;
	
	UDR0 = data;											// Vlozenie dat do buffera, odoslanie dat.
}

// Funkcia na odoslanie textoveho retazca cez USART_0:
void USART_0_Transmit_string(const char* str){
	while(*str){
		USART_0_Transmit_char(*str++);
	}
}

// Funkcia na odoslanie desiatkoveho cisla (len do troch cifier) cez USART_0:
void USART_0_Transmit_number(int num){
	if(num < 0){											// Prepocet zaporneho cisla na kladne cislo
		USART_0_Transmit_char('-');							// Odoslanie zaporneho znamienka
		num *= -1;
	}
	if(num >= 0 && num < 10){								// Jednociferne kladne cislo	[x]
		USART_0_Transmit_char(num + 0x30);
	}else if(num >= 10 && num < 100){						// Dvojciferne kladne cislo		[xx]
		USART_0_Transmit_char((num / 10) + 0x30);
		USART_0_Transmit_char((num % 10) + 0x30);
	}else if(num >= 100 && num < 1000){						// Trojciferne kladne cislo		[xxx]
		USART_0_Transmit_char((num / 100) + 0x30);
		USART_0_Transmit_char(((num % 100) / 10) + 0x30);
		USART_0_Transmit_char((num % 10) + 0x30);
	}
}

// Vyprazdnenie prijimanych udajov (Flushing the Receive Buffer):
void USART_0_Flush(void){
	uint8_t dummy;
	while(UCSR0A & (1<<RXC)){
		dummy = UDR0;
	}
	(void)dummy;											// Nepouzita premenna (osetrenie pre kompilator).
}

// Funkcia na odoslanie datumu a casu do displeja:
void RTC_Master(void){
	// upravit
	if(LED1)LED1_ON;
	if(!LED0)LED0_OFF;
	
// Citanie dat zo zariadenia DS1307:
//	uint8_t seconds = DS1307_Read(0x00);					// Cas nieje pouzity v projekte.
	uint8_t minutes = DS1307_Read(0x01);
	uint8_t hours	= DS1307_Read(0x02);
	uint8_t day_n	= DS1307_Read(0x03);
	uint8_t day		= DS1307_Read(0x04);
	uint8_t month	= DS1307_Read(0x05);
	uint8_t year	= DS1307_Read(0x06);
	
// Odoslanie dat do zariadenia NEXTION:
	USART_0_Transmit_string("yM=");							// year - rok z RTC Mastra
	USART_0_Transmit_number(BCD_to_Decimal(year));
	USART_0_Transmit_char(0xFF);
	USART_0_Transmit_char(0xFF);
	USART_0_Transmit_char(0xFF);
	
	USART_0_Transmit_string("moM=");						// month - mesiac z RTC Mastra
	USART_0_Transmit_number(BCD_to_Decimal(month));
	USART_0_Transmit_char(0xFF);
	USART_0_Transmit_char(0xFF);
	USART_0_Transmit_char(0xFF);
	
	USART_0_Transmit_string("dM=");							// date/day - datum/den z RTC Mastra
	USART_0_Transmit_number(BCD_to_Decimal(day));
	USART_0_Transmit_char(0xFF);
	USART_0_Transmit_char(0xFF);
	USART_0_Transmit_char(0xFF);
	
	USART_0_Transmit_string("hM=");							// hour - hodina z RTC Mastra
	USART_0_Transmit_number(BCD_to_Decimal(hours));
	USART_0_Transmit_char(0xFF);
	USART_0_Transmit_char(0xFF);
	USART_0_Transmit_char(0xFF);
	
	USART_0_Transmit_string("miM=");						// minute - minuta z RTC Mastra
	USART_0_Transmit_number(BCD_to_Decimal(minutes));
	USART_0_Transmit_char(0xFF);
	USART_0_Transmit_char(0xFF);
	USART_0_Transmit_char(0xFF);
	
	USART_0_Transmit_string("dnM=");						// day number - den cislo (Po az Ne) z RTC Mastra
	USART_0_Transmit_number(BCD_to_Decimal(day_n));
	USART_0_Transmit_char(0xFF);
	USART_0_Transmit_char(0xFF);
	USART_0_Transmit_char(0xFF);

	_delay_ms(10);
	LED1_OFF;
}

// Funkcia na odoslanie vnutornej teploty a vlhkosti do displeja:
void TempRH_Master(void){
	if(LED1)LED1_ON;
	if(!LED0)LED0_OFF;
	
// Odoslanie hodnoty teploty do zariadenia NEXTION:
	int temp = SHT31_ReadTemperature();
	USART_0_Transmit_string("TinM=");						// TinM - temperature interior from Master
	USART_0_Transmit_number(temp);
	USART_0_Transmit_char(0xFF);
	USART_0_Transmit_char(0xFF);
	USART_0_Transmit_char(0xFF);
	
// Odoslanie hodnoty vlhkosti do zariadenia NEXTION:
	int hum = SHT31_ReadHumidity();
	USART_0_Transmit_string("RhM=");						// RhM - relative humidity from Master
	USART_0_Transmit_number(hum);
	USART_0_Transmit_char(0xFF);
	USART_0_Transmit_char(0xFF);
	USART_0_Transmit_char(0xFF);
	
	_delay_ms(10);
	LED1_OFF;
}

// Funkcia na odoslanie teploty a rychlosti vetra na displej:
void TempWind_Outdoor(void){
	// *** Doplnit snimanie teploty a vetra z vonkajsieho prostredia *** //
	int temp = 25;											// !!! fiktivna premenna vonkajsej teploty	!!!
	int wind = 51;											// !!! fiktivna premenna rychlosti vetra	!!!

// Odoslanie hodnoty vonkajsej teploty do zariadenia NEXTION:
	USART_0_Transmit_string("Tout=");						// RhM - relative humidity from Master
	USART_0_Transmit_number(temp);
	USART_0_Transmit_char(0xFF);
	USART_0_Transmit_char(0xFF);
	USART_0_Transmit_char(0xFF);

// Odoslanie hodnoty rychlosti vetra do zariadenia NEXTION:	
	USART_0_Transmit_string("Wind=");						// Wind - speed of wind from Master
	USART_0_Transmit_number(wind);
	USART_0_Transmit_char(0xFF);
	USART_0_Transmit_char(0xFF);
	USART_0_Transmit_char(0xFF);
}

// **************************** SLAVE devices **************************** //
// Inicializacia USART_1 komunikacie:
void USART_1_Init(void){
	// Nastavenie BAUD RATE:
	UBRR1H = (unsigned char)((F_CPU/16/BAUD)-1) >> 8;
	UBRR1L = (unsigned char)((F_CPU/16/BAUD)-1);
	
	// Enable RX receiver, TX transmitter and interrupt from RXC:
	//	UCSR1B |= (1<<RXEN);								// Povolenie prijimania.
	UCSR1B |= (1<<TXEN);									// Povolenie vysielania.
	//	UCSR1B |= (1<<RXCIE);								// Povolenie prerusenia od prijimu.

	UCSR1C = (1<<USBS)|(1<<UCSZ1)|(1<<UCSZ0);				// Nastavenie frame formatu: 2stop bit, 8data bit
}

// Funkcia na odoslanie textoveho znaku (5 az 8 datoveho bitu) cez USART_1:
void USART_1_Transmit_char(unsigned char data){
	while (!(UCSR1A & (1<<UDRE)))							// Wait for empty transmit buffer.
	;

	UDR1 = data;											// Vlozenie dat do buffera, odoslanie dat.
}

// Funkcia na odoslanie desiatkoveho cisla (len do troch cifier) cez USART_1:
void USART_1_Transmit_number(int num){
	if(num < 0){											// Prepocet zaporneho cisla na kladne cislo
		USART_1_Transmit_char('-');							// Odoslanie zaporneho znamienka
		num *= -1;
	}
	if(num >= 0 && num < 10){								// Jednociferne kladne cislo	[x]
		USART_1_Transmit_char(num + 0x30);
	}else if(num >= 10 && num < 100){						// Dvojciferne kladne cislo		[xx]
		USART_1_Transmit_char((num / 10) + 0x30);
		USART_1_Transmit_char((num % 10) + 0x30);
	}else if(num >= 100 && num < 1000){						// Trojciferne kladne cislo		[xxx]
		USART_1_Transmit_char((num / 100) + 0x30);
		USART_1_Transmit_char(((num % 100) / 10) + 0x30);
		USART_1_Transmit_char((num % 10) + 0x30);
	}
}

// Funkcia na odoslanie teploty, vetra a osvetlenia do SLAVE zariadeni:
void ActualDataforSlave(void){
	int temp_in = SHT31_ReadTemperature();					// Citanie teploty zo zariadenia SHT31.

// *** Doplnit snimanie teploty a vetra z vonkajsieho prostredia *** //
	int temp_out = 25;										// !!! fiktivna premenna vonkajsej teploty			!!!
	int wind = 51;											// !!! fiktivna premenna rychlosti vetra			!!!
	int lighting_S = 0;										// !!! fiktivna premenna osvetlenia juh (south)		!!!
	int lighting_W = 0;										// !!! fiktivna premenna osvetlenia zapad (west)	!!!

	int checksum = 0xB0 + temp_in + temp_out + wind + lighting_S + lighting_W;
	
// Odoslanie dat do SLAVE zariadeni:
	USART_1_Transmit_number(0xB0);							// Kod povelu 0xB0 - Informacia pre vsetky SLAVE zariadenia.
	USART_1_Transmit_number(temp_in);						// Odoslanie hodnoty teploty.
	USART_1_Transmit_number(temp_out);						// Odoslanie hodnoty teploty exterieru.
	USART_1_Transmit_number(wind);							// Odoslanie hodnoty teploty exterieru.
	USART_1_Transmit_number(lighting_S);					// Odoslanie hodnoty osvetlenia juh.
	USART_1_Transmit_number(lighting_W);					// Odoslanie hodnoty osvetlenia zapad.
	USART_1_Transmit_number(checksum);						// Odoslanie hodnoty checksum.
}

// Prikaz 0xA1 (priame zadanie nastavenie zaluzii), pre SLAVE:
void Master_A1(unsigned char zal_id, unsigned char pozicia, unsigned char natocenie){
	int checksum = 0xB1 + zal_id + pozicia + natocenie;

	USART_1_Transmit_number(0xB1);							// kod povelu 0xB1
	USART_1_Transmit_number(zal_id);						// odoslanie ID zaluzii
	USART_1_Transmit_number(pozicia);						// odoslanie hodnoty pozicie
	USART_1_Transmit_number(natocenie);						// odoslanie hodnoty natocenia
	USART_1_Transmit_number(0x00);
	USART_1_Transmit_number(0x00);
	USART_1_Transmit_number(checksum);						// Odoslanie hodnoty checksum.			
}

// Prikaz 0xA3 (Prirad maximalnej pozicie pre zaluzie), pre SLAVE:
void Master_A3(unsigned char zal_id, unsigned char pozicia){
	int checksum = 0xB3 + zal_id + pozicia;

	USART_1_Transmit_number(0xB3);								// kod povelu 0xB3
	USART_1_Transmit_number(zal_id);							// odoslanie ID zaluzii
	USART_1_Transmit_number(pozicia);							// odoslanie hodnoty pozicie
	USART_1_Transmit_number(0x00);
	USART_1_Transmit_number(0x00);
	USART_1_Transmit_number(0x00);
	USART_1_Transmit_number(checksum);							// Odoslanie hodnoty checksum.
}

// Prikaz 0xA5 (Prirad doby pre lokalnu dobu a Master dobu), pre SLAVE:
void Master_A5(unsigned char zal_id, unsigned char min_L, unsigned char min_M){
	int checksum = 0xB5 + zal_id + min_L + min_M;

	USART_1_Transmit_number(0xB5);								// kod povelu 0xB5
	USART_1_Transmit_number(zal_id);							// odoslanie ID zaluzii
	USART_1_Transmit_number(min_L);								// odoslanie hodnoty lokalnej doby
	USART_1_Transmit_number(min_M);								// odoslanie hodnoty master doby
	USART_1_Transmit_number(0x00);
	USART_1_Transmit_number(0x00);
	USART_1_Transmit_number(checksum);							// Odoslanie hodnoty checksum.
}

// Prikaz 0xA6 (Aktivny/Neaktivny budik), pre SLAVE:
void Master_Alarm(uint8_t bud_min, uint8_t bud_hod, unsigned char bud_nat, unsigned char bud_poz, unsigned char bud_zal_id, uint8_t bud_dni){
	if(LED1)LED1_ON;
	if(!LED0)LED0_OFF;
	
	uint8_t minutes = BCD_to_Decimal(DS1307_Read(0x01));
	uint8_t hours	= BCD_to_Decimal(DS1307_Read(0x02));
	uint8_t day_n	= BCD_to_Decimal(DS1307_Read(0x03));
	
	if(bud_hod == hours && bud_min == minutes){					// ak je aktivny budik a zaroven cas je aktualny pre budik
		int checksum = 0xB6 + bud_nat + bud_poz + bud_zal_id;
		
		for(int i = 0; i < 7; i++){								// binarne citanie, Po -> Ne:
			if(bud_dni & (1 << i) && (i + 1) == day_n){			// kontrola dna budika podla RTC dna
				USART_1_Transmit_number(0xB6);					// kod povelu 0xB6
				USART_1_Transmit_number(bud_nat);				// odoslanie hodnoty natocenia
				USART_1_Transmit_number(bud_poz);				// odoslanie hodnoty pozicie
				USART_1_Transmit_number(bud_zal_id);			// odoslanie ID zaluzii
				USART_1_Transmit_number(0x00);
				USART_1_Transmit_number(0x00);
				USART_1_Transmit_number(checksum);				// Odoslanie hodnoty checksum.
			}
			_delay_ms(100);
		}
	}
	_delay_ms(10);
	LED1_OFF;
}

// Prikaz 0xA7 (Umyvanie okien),pre SLAVE:
void Master_A7(unsigned char umyva_zal_id, unsigned char umyva_on_off){
	int checksum = 0xB7 + umyva_zal_id + umyva_on_off;
	USART_1_Transmit_number(0xB7);								// kod povelu 0xB7
	USART_1_Transmit_number(umyva_zal_id);						// odoslanie ID zaluzii
	USART_1_Transmit_number(umyva_on_off);						// odoslanie hodnoty 1/0 umyvanie
	USART_1_Transmit_number(0x00);
	USART_1_Transmit_number(0x00);
	USART_1_Transmit_number(0x00);
	USART_1_Transmit_number(checksum);							// Odoslanie hodnoty checksum.
}

// Prikaz 0xA8 (Zrusenie manualneho rezimu),pre SLAVE:
void Master_A8(unsigned char manual_off){
	int checksum = 0xB8 + manual_off;
	USART_1_Transmit_number(0xB8);								// kod povelu 0xB8
	USART_1_Transmit_number(manual_off);						// odoslanie hodnoty vypnutia manualneho rezumu (0xFF z dipleja prikaz)
	USART_1_Transmit_number(0x00);
	USART_1_Transmit_number(0x00);
	USART_1_Transmit_number(0x00);
	USART_1_Transmit_number(0x00);
	USART_1_Transmit_number(checksum);							// Odoslanie hodnoty checksum.
}