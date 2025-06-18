/*
 * MASTER_TWI.c
 *	   MCU:	ATmega324PB
 * Created: 2023/2024
 *  Author: Andrej Klein, Milan Lacko
 *
 * Typ: Nastavenie TWI/I2C (SCL0[PORTC0], SDA0[PORTC1] pre DS1307 a SCL1[PORTE5], SDA1[PORTE6] pre SHT31) zbernice v MCU.
 */

#include "MASTER_TWI.h"

// Adresy senzora DS1307:
#define DS1307_WRITE		0xD0						// TWI_0/I2C adresa senzora DS1307 pre zapis.
#define DS1307_READ			0xD1						// TWI_0/I2C adresa senzora DS1307 pre citanie.

// Adresa senzora SHT31:
#define SHT31_ADDRESS		0x44						// TWI_1/I2C adresa (default) senzora SHT31.

// Adresy senzora SHT31 pre citanie dat:
#define SHT31_HIGH_TEMP		0x2C						// TWI_1/I2C adresa (MSB temperature) senzora SHT31.
#define SHT31_LOW_TEMP		0x06						// TWI_1/I2C adresa (LSB temperature) senzora SHT31.
#define SHT31_HIGH_HUM		0x24						// TWI_1/I2C adresa (MSB humidity) senzora SHT31.
#define SHT31_LOW_HUM		0x00						// TWI_1/I2C adresa (LSB humidity) senzora SHT31.

// ******************* [TWI_0/I2C] Senzor datumu a casu [DS1307] ******************* //

// Inicializacia TWI_0 komunikacie:
void TWI_0_Init(void){
	TWBR0 = ((F_CPU/SCL_CLOCK)-16)/2 >> 8;				// bit rate z 8Mhz na 100kHz prescaler = 00, hodnota TWBR musi byt viac ako 10.
	
// Nastavenie rychlosti prenosu dat na hodnotu 100KHz:
	TWSR0 &= ~(1 << TWPS0);
	TWSR0 &= ~(1 << TWPS1);
	
// Nastavenie Pull_Up pre TWI_0/I2C komunikaciu:
	DDRC &= ~((1<<PORTC0)|(1<<PORTC1));					// Logicka 0 pre SDA(PORTC0) a SCL(PORTC1) - [VSTUP] 
	PORTC = (1<<PORTC0)|(1<<PORTC1);					// Zapnutie pull_up rezistorov na SDA(PORTC0) a SCL(PORTC1) pinoch.
}

// Funkcia na odoslanie START podmienky pre TWI_0/I2C:
unsigned char TWI_0_Start(void) {
	TWCR0 = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);

// Cakanie na TWINT flag s osetrenim zacyklenia:
	int i = 0;
	while(!(TWCR0 & (1<<TWINT))){
		if(i > 1000)
			break;
		i++;
	}
	
// Kontrola hodnoty TWI_0/I2C status registra:
	if ((TWSR0 & 0xF8) != TW_START && ((TWSR0 & 0xF8) != TW_REP_START))
		return 1;
	return 0;
}

// Funkcia na odoslanie STOP podmienky pre TWI_0/I2C:
unsigned char TWI_0_Stop(void){
	TWCR0 = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
	
// Cakanie na TWSTO flag. (pocka na jej dokoncenie):
	while(!(TWCR0 & (1<<TWSTO)))
		;
		
// Kontrola hodnoty TWI_0/I2C status registra:
	if ((TWSR0 & 0xF8) != TW_SR_STOP)
		return 1;
	return 0;
}

// Funkcia na vysielanie byte-u na zbernicu, vrati 0 po uspesnom vysielani - potvrdene s ACK, vrati 1 - data nepotvrdene ACK bitom:
unsigned char TWI_0_Write(unsigned char data){
// Nacitanie DAT do TWDR registra (odosle data na predtym adresovane zariadenie):
	TWDR0 = data;
	TWCR0 = (1<<TWINT)|(1<<TWEN);						// Clear TWINT bit in TWCR to start transmission of data.

// Cakanie na TWINT flag s osetrenim zacyklenia:
	int i = 0;
	while(!(TWCR0 & (1<<TWINT))){
		if(i>1000)
			break;
		i++;
	}

// Kontrola hodnoty TWI_0/I2C status registra:
	if((TWSR0 & 0xF8) != TW_MT_DATA_ACK)
		return 1; 	
	return 0; 
}

// Funkcia na prijem byte-u zo zbernice - nie je posledny bajt, vrati prijaty bajt a nasleduje dalsi bajt:
unsigned char TWI_0_Read_Ack(void){
	TWCR0 = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);				// Clear TWINT bit in TWCR to start transmission of data.
	
// Cakanie na TWINT flag s osetrenim zacyklenia:
	int i = 0;
	while(!(TWCR0 & (1<<TWINT))){
		if(i>1000)
			break;
		i++;
	}
	return TWDR0;
}

// Funkcia na prijem byte-u zo zbernice - posledny bajt, vrati prijaty bajt a nasleduje stop bit:
unsigned char TWI_0_Read_nAck(void){
	TWCR0 = (1<<TWINT)|(1<<TWEN);						// Clear TWINT bit in TWCR to start transmission of data.
	
// Cakanie na TWINT flag s osetrenim zacyklenia:
	int i = 0;
	while(!(TWCR0 & (1<<TWINT))){
		if(i>1000)
			break;
		i++;
	}
	return TWDR0;
}

// Funkcia na prevod binarneho cisla [Binary-Coded Decimal] na cele cislo:
uint8_t BCD_to_Decimal(uint8_t bcd){
	return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

// Funkcia na prevod celeho cisla na binarne cislo [Binary-Coded Decimal]:
uint8_t Decimal_to_BCD(uint8_t decimal){
	uint8_t tens = decimal / 10;						// Ziskanie desiatok.
	uint8_t units = decimal % 10;						// Ziskanie jednotiek.
	return (tens << 4) | units;							// Spojenie desiatok a jednotiek v BCD formate.
}

// Funkcia na zapis datumu a casu do senzora DS1307:
void DS1307_Write(uint8_t address, uint8_t data) {
	TWI_0_Start();
	TWI_0_Write(DS1307_WRITE);
	TWI_0_Write(address);
	TWI_0_Write(data);
	TWI_0_Stop();
}

// Funkcia na citanie datumu a casu zo senzora DS1307:
uint8_t DS1307_Read(uint8_t address) {
	uint8_t data;
	TWI_0_Start();
	TWI_0_Write(DS1307_WRITE);
	TWI_0_Write(address);
	TWI_0_Start();
	TWI_0_Write(DS1307_READ);
	data = TWI_0_Read_nAck();
	TWI_0_Stop();
	return data;
}

// Funkcia na nastavenie datumu a casu pomocou funkcie zapisu DS1307:
void DS1307_Set_RTC(uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t day_n){
	if(!LED1)LED1_ON;
	if(LED0)LED0_OFF;
	
	DS1307_Write(0x00, Decimal_to_BCD(0));				// Nastavenie sekundy
	DS1307_Write(0x01, Decimal_to_BCD(minute));			// Nastavenie minuty
	DS1307_Write(0x02, Decimal_to_BCD(hour));			// Nastavenie hodiny (24-hodinovy format)
	DS1307_Write(0x03, Decimal_to_BCD(day_n));			// Nastavenie dna (Pondelok az Nedela)
	DS1307_Write(0x04, Decimal_to_BCD(day));			// Nastavenie datumu dna
	DS1307_Write(0x05, Decimal_to_BCD(month));			// Nastavenie mesiaca
	DS1307_Write(0x06, Decimal_to_BCD(year));			// Nastavenie roka
	
	_delay_ms(50);
	LED1_OFF;
}

// ******************* [TWI_1/I2C] Senzor teploty a vlhkosti [SHT31] ******************* //

// Inicializacia TWI_1 komunikacie:
void TWI_1_Init(void){
	TWBR1 = ((F_CPU/SCL_CLOCK)-16)/2;					// bit rate z 8Mhz na 100kHz prescaler = 00, hodnota TWBR musi byt viac ako 10.
	
	// Nastavenie rychlosti prenosu dat na hodnotu 100KHz:
	TWSR1 &= ~(1 << TWPS0);
	TWSR1 &= ~(1 << TWPS1);
	
	// Nastavenie Pull_Up pre TWI_1/I2C komunikaciu:
	DDRE &= ~((1<<PORTE5)|(1<<PORTE6));					// Logicka 0 pre SDA(PORTE5) a SCL(PORTE6) - [VSTUP]
	PORTE = (1<<PORTE5)|(1<<PORTE6);					// Zapnutie pull_up rezistorov na SDA(PORTE5) a SCL(PORTE6) pinoch.
}

// Funkcia na odoslanie START podmienky pre TWI_1/I2C:
unsigned char TWI_1_Start(void) {
	TWCR1 = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);

	// Cakanie na TWINT flag s osetrenim zacyklenia:
	int i = 0;
	while(!(TWCR1 & (1<<TWINT))){
		if(i > 1000)
		break;
		i++;
	}
	
	// Kontrola hodnoty TWI_1/I2C status registra:
	if ((TWSR1 & 0xF8) != TW_START && ((TWSR1 & 0xF8) != TW_REP_START))
		return 1;
	return 0;
}

// Funkcia na odoslanie STOP podmienky pre TWI_1/I2C:
unsigned char TWI_1_Stop(void){
	TWCR1 = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
	
	// Cakanie na TWSTO flag. (pocka na jej dokoncenie):
	while(!(TWCR1 & (1<<TWSTO)))
	;
	
	// Kontrola hodnoty TWI_1/I2C status registra:
	if ((TWSR1 & 0xF8) != TW_SR_STOP)
		return 1;
	return 0;
}

// Funkcia na vysielanie byte-u na zbernicu, vrati 0 po uspesnom vysielani - potvrdene s ACK, vrati 1 - data nepotvrdene ACK bitom:
unsigned char TWI_1_Write(unsigned char data){
	// Nacitanie DAT do TWDR registra (odosle data na predtym adresovane zariadenie):
	TWDR1 = data;
	TWCR1 = (1<<TWINT)|(1<<TWEN);						// Clear TWINT bit in TWCR to start transmission of data.

	// Cakanie na TWINT flag s osetrenim zacyklenia:
	int i = 0;
	while(!(TWCR1 & (1<<TWINT))){
		if(i>1000)
			break;
		i++;
	}

	// Kontrola hodnoty TWI_1/I2C status registra:
	if((TWSR1 & 0xF8) != TW_MT_DATA_ACK)
		return 1;
	return 0;
}

// Funkcia na prijem byte-u zo zbernice - nie je posledny bajt, vrati prijaty bajt a nasleduje dalsi bajt:
unsigned char TWI_1_Read_Ack(void){
	TWCR1 = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);				// Clear TWINT bit in TWCR to start transmission of data.
	
	// Cakanie na TWINT flag s osetrenim zacyklenia:
	int i = 0;
	while(!(TWCR1 & (1<<TWINT))){
		if(i>1000)
			break;
		i++;
	}
	return TWDR1;
}

// Funkcia na prijem byte-u zo zbernice - posledny bajt, vrati prijaty bajt a nasleduje stop bit:
unsigned char TWI_1_Read_nAck(void){
	TWCR1 = (1<<TWINT)|(1<<TWEN);						// Clear TWINT bit in TWCR to start transmission of data.
	
	// Cakanie na TWINT flag s osetrenim zacyklenia:
	int i = 0;
	while(!(TWCR1 & (1<<TWINT))){
		if(i>1000)
			break;
		i++;
	}
	return TWDR1;
}

// Funkcia na inicializaciu senzora SHT31:
void SHT31_Init(void){
	TWI_1_Init();
}

// Funkcia na citanie teploty zo senzora SHT31:
float SHT31_ReadTemperature(void) {
	uint16_t temperature_raw = 0;	
	TWI_1_Start();										// Zaciatok komunikacie.
	
	// Adresa snimaca s prikazom na meranie teploty:
	TWI_1_Write(SHT31_ADDRESS << 1);
	TWI_1_Write(SHT31_HIGH_TEMP);
	TWI_1_Write(SHT31_LOW_TEMP);	
	_delay_ms(20);										// Cakanie na spracovanie udajov.
	
	// Citanie udajov:
	TWI_1_Start();
	TWI_1_Write((SHT31_ADDRESS << 1) | 0x01);
	temperature_raw = TWI_1_Read_nAck() << 8;
	temperature_raw |= TWI_1_Read_nAck();
	
	TWI_1_Stop();										// Ukoncenie komunikacie.	
	float temperature = -45.0 + 175.0 * temperature_raw / 65535.0;		// Prepocet vysledku podla vzorca na teplotu.
	return temperature;
}

// Funkcia na citanie vlhkosti zo senzora SHT31:
float SHT31_ReadHumidity(void) {
	uint16_t humidity_raw = 0;
	TWI_1_Start();										// Zaciatok komunikacie.
	
	// Adresa snimaca s prikazom na meranie vlhkosti:
	TWI_1_Write(SHT31_ADDRESS << 1);
	TWI_1_Write(SHT31_HIGH_HUM);
	TWI_1_Write(SHT31_LOW_HUM);
	_delay_ms(20);										// Cakanie na spracovanie udajov.
	
	// Citanie udajov:
	TWI_1_Start();
	TWI_1_Write((SHT31_ADDRESS << 1) | 0x01);
	humidity_raw = TWI_1_Read_nAck() << 8;
	humidity_raw |= TWI_1_Read_nAck();
	
	TWI_1_Stop();										// Ukoncenie komunikacie.
	float humidity = 100.0 * humidity_raw / 65535.0;		// Prepocet vysledku podla vzorca na vlhkost.
	return humidity;
}