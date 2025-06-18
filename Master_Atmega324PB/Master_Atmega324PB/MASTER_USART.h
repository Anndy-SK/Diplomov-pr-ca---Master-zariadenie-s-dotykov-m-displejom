/*
 * MASTER_USART.h
 * 	   MCU:	ATmega324PB
 * Created: 2023/2024
 *  Author: Andrej Klein, Milan Lacko
 *
 * Typ: Nastavenie USART_0 (RXD0[PORTD0], TXD0[PORTD1]) zbernice v MCU a komunikacie s NEXTION display-om.
 */ 

/*
    // KODY PRIKAZOV z displeja (NEXTION) ako 1. bajt spravy:
	//  161 - 0xA1 priame zadanie nastavenie zaluzii - Master zadanie - potom plynie master doba
	//  163 - 0xA3 Prirad maximalnej pozicie pre zaluzie
	//  165 - 0xA5 Prirad doby pre lokalnu dobu a Master dobu
	//  167 - 0xA7 Umyvanie okien
	//  168 - 0xA8 Zrusenie manualneho rezimu
	//  169 - 0xA9 Nastavenie datumu a casu
	//  171 - 0xAB Aktivny/Neaktivny budik1
	//  173 - 0xAD Aktivny/Neaktivny budik2
	//  175 - 0xAF Aktivny/Nneaktivny budik3

	USART_0_DATA[0] - kod povelu  |	161 - 0xA1		163 0xA3		165 0xA5		167 0xA7		168 0xA8		169 0xA9		171 0xAB		173 0xAD		175 0xAF  
	USART_0_DATA[1] -			  |	zaluzieID		zaluzieID		zaluzieID		zaluzieID		zaluzieID		rok				1/0 aktiv		1/0 aktiv		1/0 aktiv
	USART_0_DATA[2] -			  |	pozicia			pozicia			minuty_L		1/0 umyvanie		0			mesiac			min				min				min
	USART_0_DATA[3] -			  |	natocenie	    	0			minuty_M		   0				0			den				hod				hod				hod
	USART_0_DATA[4] -			  |	   0				0			   0			   0				0			hodiny			nat				nat				nat	
	USART_0_DATA[5] -			  |    0				0			   0			   0				0			minuty			poz				poz				poz
	USART_0_DATA[6] -			  |	   0				0			   0			   0				0			den (po-pi)		zaluzieID		zaluzieID		zaluzieID
	USART_0_DATA[7] -			  |	   0				0			   0			   0				0			0				dni				dni				dni
	USART_0_DATA[8] - CHCKSUM	  |	CHCKSUM			CHCKSUM			CHCKSUM			CHCKSUM			CHCKSUM			CHCKSUM			CHCKSUM			CHCKSUM			CHCKSUM

	// KODY PRIKAZOV pre SLAVE zariadenie ako 1. bajt spravy:
	//	176 - 0xB0 Aktualne informacie zo zariadenia Master pre Slave zariadenia (teplota interieru a exterieru; vietor; osvetlenie Juh a Zapad)
	//  177 - 0xB1 Priame zadanie nastavenie zaluzii - Master zadanie - potom plynie master doba
	//  179 - 0xB3 Prirad maximalnej pozicie pre zaluzie
	//  181 - 0xB5 Prirad doby pre lokalnu dobu a Master dobu
	//  182 - 0xB6 Budik	
	//  183 - 0xB7 Umyvanie okien
	//  184 - 0xB8 Zrusenie manualneho rezimu


	USART_1_DATA[0] - KOD povelu |		176 - 0xB0		177 0xB1		179 0xB3		181 0xB5		182 0xB6		183 0xB7		184 0xB8
	USART_1_DATA[1] - Data 1	 |		teplota Int		zaluzieID		zaluzieID		zaluzieID		bud_nat			zaluzieID		zaluzieID
	USART_1_DATA[2] - Data 2	 |		teplota Ext		pozicia			pozicia			minuty_L		bud_poz			1/0 umyvanie		0
	USART_1_DATA[3] - Data 3	 |		vietor			natocenie			0			minuty_M		bud_zal_id			0				0
	USART_1_DATA[4] - Data 4	 |		osvetl. J			0				0				0				0				0				0
	USART_1_DATA[5] - Data 5	 |		osvetl. Z			0				0				0				0				0				0
	USART_1_DATA[6] - CHCKSUM	 |		 CHCKSUM		 CHCKSUM		 CHCKSUM		 CHCKSUM		 CHCKSUM		 CHCKSUM		 CHCKSUM

*/

#ifndef MASTER_USART_H_
#define MASTER_USART_H_

#include "F_CPU.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include "MASTER_GPIO.h"
#include "MASTER_TWI.h"

#define BAUD 9600											// Prenosova rychlost USART komunikacie.
#define ARRAY_SIZE 9										// Definicia velkosti pola pre prijem dat z displeja.

volatile char UART_0_DATA[ARRAY_SIZE];						// Definicia pola pre prichadzajuce spravy z displeja (MASTER DATA).

// ******************* NEXTION ******************* //
// Deklaracie funkcii komunikacie USART_0 (RXD0, TXD0) pre NEXTION displej:
extern void USART_0_Init(void);								// Inicializacia USART_0 komunikacie.
extern void USART_0_Transmit_char(unsigned char data);		// Funkcia na odoslanie textoveho znaku (5 az 8 datoveho bitu) cez USART_0.
extern void USART_0_Transmit_string(const char* str);		// Funkcia na odoslanie textoveho retazca cez USART_0.
extern void USART_0_Transmit_number(int num);				// Funkcia na odoslanie desiatkoveho cisla (len do dvoch cifier) cez USART_0.
extern void USART_0_Flush(void);							// Vyprazdnenie prijimanych udajov (Flushing the Receive Buffer).

// Deklaracie funkcii pre zapis datumu, casu a teploty po USART_0 komunikacii na displej:
extern void RTC_Master(void);								// Funkcia na odoslanie datumu a casu do displeja.
extern void TempRH_Master(void);							// Funkcia na odoslanie vnutornej teploty a vlhkosti do displeja.

// Deklaracia funkcie pre odoslanie teploty a rychlosti vetra na displej:
extern void TempWind_Outdoor(void);

// ******************* SLAVE devices ******************* //
// Deklaracie funkcii komunikacie USART_1 (RXD1, TXD1) pre SLAVE zariadenia:
extern void USART_1_Init(void);								// Inicializacia USART_1 komunikacie.
extern void USART_1_Transmit_char(unsigned char data);		// Funkcia na odoslanie textoveho znaku (5 az 8 datoveho bitu) cez USART_1.
extern void USART_1_Transmit_number(int num);				// Funkcia na odoslanie desiatkoveho cisla (len do dvoch cifier) cez USART_1.

// Deklaracie funkcii pre SLAVE zariadenia:
extern void ActualDataforSlave(void);						// Funkcia na odoslanie teploty, vetra a osvetlenia do SLAVE zariadeni.

extern void Master_A1(unsigned char zal_id, unsigned char pozicia, unsigned char natocenie);			// Prikaz 0xA1 (priame zadanie nastavenie zaluzii), pre SLAVE.
extern void Master_A3(unsigned char zal_id, unsigned char pozicia);										// Prikaz 0xA3 (Prirad maximalnej pozicie pre zaluzie), pre SLAVE.
extern void Master_A5(unsigned char zal_id, unsigned char min_L, unsigned char min_M);					// Prikaz 0xA5 (Prirad doby pre lokalnu dobu a Master dobu), pre SLAVE.

// Deklaracia funkcii pre vsetky budiky:
extern void Master_Alarm(uint8_t bud_min, uint8_t bud_hod, unsigned char bud_nat, unsigned char bud_poz, unsigned char bud_zal_id, uint8_t bud_dni);		// Prikaz 0xA6 (Aktivny/Neaktivny budik), pre SLAVE.

extern void Master_A7(unsigned char umyva_zal_id, unsigned char umyva_on_off);							// Prikaz 0xA7 (Umyvanie okien),pre SLAVE.
extern void Master_A8(unsigned char manual_off);														// Prikaz 0xA8 (Zrusenie manualneho rezimu),pre SLAVE.


#endif /* MASTER_USART_H_ */