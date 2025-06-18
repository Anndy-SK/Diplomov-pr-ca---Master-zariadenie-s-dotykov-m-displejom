/*
 * main.c
 * 	   MCU:	ATmega324PB
 * Created: 2023/2024
 *  Author: Andrej Klein, Milan Lacko
 *
 * Typ: Hlavny program
 */

#include "MASTER_GPIO.h"
#include "MASTER_TWI.h"
#include "MASTER_USART.h"

// Definovanie premennych pre USART_0 [prijem dat z displeja]:
volatile unsigned char P_UART_0 = 0;
volatile unsigned int poc = 0;

// Definovanie premennych podla povelu z displeja:
volatile unsigned char zadanie_zal_ID, zadanie_pozicia, zadanie_natocenie;										// nastavenie -priame zadanie - nastavenie zaluzii
volatile unsigned char max_zal_ID, max_pozicia;																	// priradenie maximalnej pozicie
volatile unsigned char doby_zal_ID, doby_min_L, doby_min_M;														// priradenie master doby a lokalnej doby
volatile unsigned char umyva_zal_ID, umyva_ON_OFF;																// umyvanie
volatile unsigned char manual_OFF;																				// manualny rezim
volatile unsigned char nastav_rok, nastav_mesiac, nastav_den, nastav_hod, nastav_min, nastav_den_n = 0;			// nastavenie datumu a casu
volatile unsigned char bud1_ON_OFF, bud1_min, bud1_hod, bud1_nat, bud1_poz, bud1_zal_ID, bud1_dni = 0;				// budik 1
volatile unsigned char bud2_ON_OFF, bud2_min, bud2_hod, bud2_nat, bud2_poz, bud2_zal_ID, bud2_dni = 0;				// budik 2
volatile unsigned char bud3_ON_OFF, bud3_min, bud3_hod, bud3_nat, bud3_poz, bud3_zal_ID, bud3_dni = 0;				// budik 3

// Nastavenie minuty na hodnotu 61, pre spustenie funkcie (minutes != last_min), "hodnota 0 - 60" nemusi spustit citanie RTC prvykrat po spusteni kedze sa moze rovnat:
uint8_t last_min = 61;					

// Deklaracia funkcie pre prijem dat z displeja:
void UlozDatazDispleja (void);

int main(void){
	LD_SW_init();													// Inicializacia LED0, LED1 a tlacidiel SW0, SW1.
	TWI_0_Init();													// Inicializacia TWI_0 komunikacie (DS1307) - bitrate - 100kHz
	TWI_1_Init();													// Inicializacia TWI_1 komunikacie (SHT31)  - bitrate - 100kHz
	SHT31_Init();													// Inicializacia TWI_1 pre senzor SHT31
	USART_0_Init();													// Inicializacia USART_0 komunikacie (NEXTION) - 2stop bit, 8data bit
	USART_1_Init();													// Inicializacia USART_1 komunikacie (SLAVE)   - 2stop bit, 8data bit

//	DS1307_Set_RTC(24,4,6,19,40,6);									// Inicializacia datumu a casu [rok, mesiac, den, hodiny, minuty, den(Po-Ne)] - pouzit len na striktne nastavenie (prvykrat).

	sei();															// Zapnutie globalnych preruseni.

	Master_start();													// Signalizacia pomocou LED ze master zacal loop.
	
    while(1){
//		DS1307_RTC_test();											// Test datumu a casu (DS1307 senzora) pomocou tlacidka SW0.
//		SHT31_TempRH_test();										// Test teploty a vlhkosti (SHT31 senzora) pomocou tlacidka SW1.
		
		if(LED0)LED0_ON;											// Kontrolka LED necinnosti.

		if (P_UART_0 != 9 && P_UART_0 != 0){						// Ked nic neprichadza alebo este neprisli vsetky bajty.
			poc++;													// Ak prijime menej ako 9 bajtov, alebo viac ako 9, niekde problem...po case zmaz pocitadlo P_UART_0. Ak je P_UART_0 = 0, znamena, ze uz dlhsiu dobu neboli poslane ziadne data.
			if (poc > 65000){										// Ak su poslane data, tak presne 9 bajtov - ak sa nic nemeni dlhsiu dobu > 65000 urob toto.
				LED0_ON;
				P_UART_0 = 0;										// Nastav pocitadlo dat na 0 a zahod vsetky data.
				poc = 0;											// Znuluj pocitadlo neprijimanie bajtov.
			}
		}
		
		if (P_UART_0 == 9){											// Ak je uz prijatych vsetkych 9 bajtov.
			_delay_ms(100);
			char temp;
			temp = UART_0_DATA[0]+UART_0_DATA[1]+UART_0_DATA[2]+UART_0_DATA[3]+UART_0_DATA[4]+UART_0_DATA[5]+UART_0_DATA[6]+UART_0_DATA[7];
			if (temp == UART_0_DATA[8]){							// Porovnanie suctu s CHECKSUM.
				_delay_ms(100);
				
				// Zobrazenie stavu "Cakajte.....hotovo" v displeji:
				USART_0_Transmit_string("vis t0,");					
				USART_0_Transmit_char(0x31);
				USART_0_Transmit_char(0xFF);
				USART_0_Transmit_char(0xFF);
				USART_0_Transmit_char(0xFF);
				_delay_ms(600);
				// Skrytie stavu "Cakajte.....hotovo" v displeji:
				USART_0_Transmit_string("vis t0,");
				USART_0_Transmit_char(0x30);
				USART_0_Transmit_char(0xFF);
				USART_0_Transmit_char(0xFF);
				USART_0_Transmit_char(0xFF);
				_delay_ms(50);
											
				UlozDatazDispleja();								// Nacitanie dat z displeja do premennych pre dalsie pokyny.
			}else{
				// Ak sa zobudi display z necinnosti nahraj data z mastra do displeja:
				for(int i = 0; i < 3; i++){
					if(UART_0_DATA[0] == 0x87 || UART_0_DATA[0] == 0x88 || UART_0_DATA[0] == 0x68){
						RTC_Master();								// Odoslanie aktualneho datumu a casu do displeja.
						TempRH_Master();							// Odoslanie vnutornej teploty a vlhkosti do displeja.
						TempWind_Outdoor();							// Odoslanie vonkajsej teploty a rychlosti vetra do displeja.
						// Prikaz na aktualizaciu dat pomocou "page 0", ("ref 0" - nefunguje spravne):
						USART_0_Transmit_string("page 0");			
						USART_0_Transmit_char(0xFF);
						USART_0_Transmit_char(0xFF);
						USART_0_Transmit_char(0xFF);
						break;
					}
				}
			}
			P_UART_0 = 0;											// Nastav pocitadlo dat na 0 a zahod vsetky data.
		}

		// Podmienka ktora kazdu minutu aktualizuje datum, cas, teplotu a vlhkost, kontroluje budiky a posiela data do SLAVE zariadeni:
		uint8_t act_min = BCD_to_Decimal(DS1307_Read(0x01));		// Adresa "0x01" je aktualna hodnota pre minuty v RTC/DS1307.
		if(act_min != last_min){									// Ak sa prebehla minuta, tak aktualizuj data
			last_min = BCD_to_Decimal(DS1307_Read(0x01));			// Aktualizovanie premennej pre poslednu nacitanu minutu z DS1307.
			
			RTC_Master();											// Odoslanie aktualneho datumu a casu do displeja.
			TempRH_Master();										// Odoslanie vnutornej teploty a vlhkosti do displeja.
			TempWind_Outdoor();										// Odoslanie vonkajsej teploty a rychlosti vetra do displeja.
			ActualDataforSlave();									// Odoslanie teploty, vetra a osvetlenia do SLAVE zariadeni.
			_delay_ms(100);
			if(bud1_ON_OFF){										// Ak je aktivny status pre budik 1.
				Master_Alarm(bud1_min, bud1_hod, bud1_nat, bud1_poz, bud1_zal_ID, bud1_dni);
				_delay_ms(50);
			}
			if(bud2_ON_OFF){										// Ak je aktivny status pre budik 2.
				Master_Alarm(bud2_min, bud2_hod, bud2_nat, bud2_poz, bud2_zal_ID, bud2_dni);
				_delay_ms(50);
			}
			if(bud3_ON_OFF){										// Ak je aktivny status pre budik 3.
				Master_Alarm(bud3_min, bud3_hod, bud3_nat, bud3_poz, bud3_zal_ID, bud3_dni);
				_delay_ms(50);
			}
						
		}
	
    }	// Koniec "while(1)".
	
	return 0;
}	// Koniec "main".

void UlozDatazDispleja (void)
{
	if(LED1)LED1_ON;
	if(!LED0)LED0_OFF;
	
	switch (UART_0_DATA[0])
	{
		// Priame zadanie nastavenia zaluzii:
		case 0xA1:
			zadanie_zal_ID		= UART_0_DATA[1];
			zadanie_pozicia		= UART_0_DATA[2];
			zadanie_natocenie	= UART_0_DATA[3];
			Master_A1(zadanie_zal_ID, zadanie_pozicia, zadanie_natocenie);
			break;
		// Priradenie maximalnej pozicie zaluzie:
		case 0xA3:
			max_zal_ID		= UART_0_DATA[1];
			max_pozicia		= UART_0_DATA[2];
			Master_A3(max_zal_ID, max_pozicia);
			break;
		// Priradenie doby zaluzie - master doby a lokalnej doby:
		case 0xA5:
			doby_zal_ID		= UART_0_DATA[1];
			doby_min_L		= UART_0_DATA[2];
			doby_min_M		= UART_0_DATA[3];
			Master_A5(doby_zal_ID, doby_min_L, doby_min_M);
			break;
		// Umyvanie zaluzii:
		case 0xA7:
			umyva_zal_ID	= UART_0_DATA[1];
			umyva_ON_OFF	= UART_0_DATA[2];
			Master_A7(umyva_zal_ID, umyva_ON_OFF);
			break;
		// Zrusenie manualneho rezimu:
		case 0xA8:
			manual_OFF	= UART_0_DATA[1];
			Master_A8(manual_OFF);
			break;
		// Nastavenie datumu a casu:
		case 0xA9:
			nastav_rok		= UART_0_DATA[1];
			nastav_mesiac	= UART_0_DATA[2];
			nastav_den		= UART_0_DATA[3];
			nastav_hod		= UART_0_DATA[4];
			nastav_min		= UART_0_DATA[5];
			nastav_den_n	= UART_0_DATA[6];
			DS1307_Set_RTC(nastav_rok, nastav_mesiac, nastav_den, nastav_hod, nastav_min, nastav_den_n);
			RTC_Master();
			// Zmena strany v displeji na page 0:
			USART_0_Transmit_string("page 0");
			USART_0_Transmit_char(0xFF);
			USART_0_Transmit_char(0xFF);
			USART_0_Transmit_char(0xFF);
			break;
		// Budik 1:
		case 0xAB:
			bud1_ON_OFF	= UART_0_DATA[1];
			bud1_min	= UART_0_DATA[2];
			bud1_hod	= UART_0_DATA[3];
			bud1_nat	= UART_0_DATA[4];
			bud1_poz	= UART_0_DATA[5];
			bud1_zal_ID	= UART_0_DATA[6];
			bud1_dni	= UART_0_DATA[7];
			break;
		// Budik 2:
		case 0xAD:
			bud2_ON_OFF	= UART_0_DATA[1];
			bud2_min	= UART_0_DATA[2];
			bud2_hod	= UART_0_DATA[3];
			bud2_nat	= UART_0_DATA[4];
			bud2_poz	= UART_0_DATA[5];
			bud2_zal_ID	= UART_0_DATA[6];
			bud2_dni	= UART_0_DATA[7];
			break;
		// Budik 3:
		case 0xAF:
			bud3_ON_OFF	= UART_0_DATA[1];
			bud3_min	= UART_0_DATA[2];
			bud3_hod	= UART_0_DATA[3];
			bud3_nat	= UART_0_DATA[4];
			bud3_poz	= UART_0_DATA[5];
			bud3_zal_ID	= UART_0_DATA[6];
			bud3_dni	= UART_0_DATA[7];
			break;
	}
	_delay_ms(50);
	LED1_OFF;
}


// Prerusenie od prijimu USART0 RXC - prijem od displeja:
ISR(USART0_RX_vect, ISR_NAKED){
	asm ("in r15, __SREG__");
	asm ("push r15");
	asm ("push r24");
	asm ("push r25");
	asm ("push r30");
	asm ("push r31");
	
	UART_0_DATA[P_UART_0] = UDR0;						// Uloz prijate data.
	P_UART_0++;											// Inkrementuj pocitadlo - budem z displeja posielat stale 9 bajtov.
	poc = 0;											// Po prijati bajtu, zmaz pocitadlo necinnosti.

	USART_0_Flush();									// Vyprazdnenie prijimanych udajov (Flushing the Receive Buffer).
	
	asm ("pop r31");
	asm ("pop r30");
	asm ("pop r25");									// Vytiahni r25 zo zasobnika.
	asm ("pop r24");									// Vytiahni r24 zo zasobnika.
	asm ("pop r15");									// Vytiahni Status register SREG kvoli Carry flag ked sa porovnava - zo zasobnika cez r15 - r15 nie je nikde v hlavnom prog. pouzita.
	asm("out __SREG__ , r15");
	
	reti();												// Navrat spat do programu.
}