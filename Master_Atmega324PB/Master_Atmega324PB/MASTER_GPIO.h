/*
 * MASTER_GPIO.h
 *	   MCU:	ATmega324PB
 * Created: 2023/2024
 *  Author: Andrej Klein, Milan Lacko
 *
 * Typ: Nastavenie GPIO a praca s GPIO.
 */

#ifndef MASTER_GPIO_H_
#define MASTER_GPIO_H_

#include "F_CPU.h"								// Frekvencia CPU (ATmega324PB - 8MHz).
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "MASTER_USART.h"
#include "MASTER_TWI.h"

#define LED0_ON		(PORTD &= ~(1<<PORTD4))		// LED0 - zapnuta - logicka 0 je pripojena na GND - POZOR !!! toto je defaultna hodnota !!!
#define LED0_OFF	(PORTD |= (1<<PORTD4))		// LED0 - vypnuta - logicka 1.
#define LED0		(PIND & (1<<PORTD4))		// LED0 indikacia - vycitanie pinu.

#define LED1_ON		(PORTD &= ~(1<<PORTD5))		// LED1 - zapnuta - logicka 0 je pripojena na GND - POZOR !!! toto je defaultna hodnota !!!
#define LED1_OFF	(PORTD |= (1<<PORTD5))		// LED1 - vypnuta - logicka 1.
#define LED1		(PIND & (1<<PORTD5))		// LED1 indikacia - vycitanie pinu.

#define SW0			(PIND & (1<<PORTD6))		// Definicia tlacidla SW0.
#define SW1			(PIND & (1<<PORTD7))		// Definicia tlacidla SW1.

extern void Master_start(void);					// Indikator zaciatku behu programu (loopu) pre MASTER.
extern void LD_SW_init(void);					// Nastavenie portov pre LED a tlacidla.
extern void DS1307_RTC_test(void);				// Funkcia pre testy s RTC [DS1307].
extern void SHT31_TempRH_test(void);			// Funkcia pre testy teploty a vlhkosti [SHT31].

#endif /* MASTER_GPIO_H_ */