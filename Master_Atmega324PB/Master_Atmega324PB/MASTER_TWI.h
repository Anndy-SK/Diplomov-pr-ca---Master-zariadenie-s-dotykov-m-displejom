/*
 * MASTER_TWI.h
 *	   MCU:	ATmega324PB
 * Created: 2023/2024
 *  Author: Andrej Klein, Milan Lacko
 *
 * Typ: Nastavenie TWI/I2C (SCL0[PORTC0], SDA0[PORTC1] pre DS1307 a SCL1[PORTE5], SDA1[PORTE6] pre SHT31) zbernice v MCU.
 */ 

#ifndef MASTER_TWI_H_
#define MASTER_TWI_H_

#include "F_CPU.h"
#include <avr/io.h>
#include <util/twi.h>
#include "MASTER_GPIO.h"

#define SCL_CLOCK   100000L										// Prenosova rychlost TWI/I2C = 100KHz (max: 400KHz).

// ******************* Senzor datumu a casu [DS1307] ******************* //
// Deklaracie funkcii komunikacie TWI_0/I2C (SDA0, SCL0) pre RTC senzor datumu a casu [DS1307]:
extern void TWI_0_Init(void);									// Inicializacia TWI_0 komunikacie.
extern unsigned char TWI_0_Start(void);							// Funkcia na odoslanie START podmienky.
extern unsigned char TWI_0_Stop(void);							// Funkcia na odoslanie STOP podmienky.
extern unsigned char TWI_0_Write(unsigned char data);			// Funkcia na vysielanie byte-u na zbernicu.
extern unsigned char TWI_0_Read_Ack(void);						// Funkcia na prijem byte-u zo zbernice - nie je posledny bajt.
extern unsigned char TWI_0_Read_nAck(void);						// Funkcia na prijem byte-u zo zbernice - posledny bajt.

// Deklaracie funkcii potrebnych pre spravne citanie a zapis senzora DS1307:
extern uint8_t BCD_to_Decimal(uint8_t bcd);						// Funkcia na prevod binarneho cisla [Binary-Coded Decimal] na cele cislo.
extern uint8_t Decimal_to_BCD(uint8_t decimal);					// Funkcia na prevod celeho cisla na binarne cislo [Binary-Coded Decimal].

// Deklaracie funkcii pre DS1307:
extern void DS1307_Write(uint8_t address, uint8_t data);		// Funkcia na zapis datumu a casu do senzora DS1307.
extern uint8_t DS1307_Read(uint8_t address);					// Funkcia na citanie datumu a casu zo senzora DS1307.
extern void DS1307_Set_RTC(uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t day_n);		// Funkcia na nastavenie datumu a casu pomocou funkcie zapisu DS1307.

// ******************* Senzor teploty a vlhkosti [SHT31] ******************* //
// Deklaracie funkcii komunikacie TWI_1/I2C (SDA1, SCL1) pre senzor teploty/vlhkosti [SHT31]:
extern void TWI_1_Init(void);									// Inicializacia TWI_1 komunikacie.
extern unsigned char TWI_1_Start(void);							// Funkcia na odoslanie START podmienky.
extern unsigned char TWI_1_Stop(void);							// Funkcia na odoslanie STOP podmienky.
extern unsigned char TWI_1_Write(unsigned char data);			// Funkcia na vysielanie byte-u na zbernicu.
extern unsigned char TWI_1_Read_Ack(void);						// Funkcia na prijem byte-u zo zbernice - nie je posledny bajt.
extern unsigned char TWI_1_Read_nAck(void);						// Funkcia na prijem byte-u zo zbernice - posledny bajt.

// Deklaracie funkcii pre SHT31:
extern void SHT31_Init(void);									// Funkcia na inicializaciu senzora SHT31.
extern float SHT31_ReadTemperature(void);						// Funkcia na citanie teploty zo senzora SHT31.
extern float SHT31_ReadHumidity(void);							// Funkcia na citanie vlhkosti zo senzora SHT31.

#endif /* MASTER_TWI_H_ */