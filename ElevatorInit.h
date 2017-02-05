#include "CommonHeader.h"
#include "Speaker.h"
#include "lcd.h"
#include "SevenSegment.h"
#include "DotMatrix.h"
#include "LED.h"
#include "SteppingMotor.h"

typedef enum {false, true} bool;

//////////////////////////////////////////// interface //////////////////////////////////////////////

// private function
static void port_init(void);
static void init_devices(void);

// public function
extern void Init(void);

//////////////////////////////////////////// --------- //////////////////////////////////////////////

static void port_init(void) {
	PORTA = 0x00;
	DDRA = 0x00;
	PORTB = 0x00;
	DDRB = 0x00; // Dependency: KeySwitch
	PORTC = 0x00;
	DDRC = 0x00;
	PORTD = 0xFF;
	DDRD = 0xFF;
	PORTE = 0x00;
	DDRE = 0x00;
	PORTF = 0x00;
	DDRF= 0x00;
	PORTG = 0x00;
	DDRG = 0x10; // Dependency: Speaker
}

static void init_devices(void){
	cli();
	XDIV = 0x00;
	XMCRA = 0x00;

	port_init();
	timer1_init(); // Dependency: Speaker

//	TCCR0 = 0x07; // TIMER0 Prescaler 1024
//	TCNT0 = 0x00; // TIMER0

//	TCCR2 = 0x07; // TIMER2 Prescaler 1024
//	TCNT2 = 0x00; // TIMER2

	MCUCR = 0x80; // Dependency: DotMatrix, SevenSegment
	EICRA = 0x00;
	EICRB = 0x00;
	EIMSK = 0x00;
	TIMSK = 0x14; // Dependency: Speaker // TOIE2(TIMER2)
//	TIMSK = 0x15; // Dependency: Speaker // TOIE0(TIMER0) // TOIE2(TIMER2)
	ETIMSK = 0X00;
	sei();
}

static void init_training_kit(void){ // zeroing whatever that lights
	EX_SS_DATA = 0; // Dependency: SevenSegment
	EX_SS_SEL = 0; // Dependency: SevenSegment
	EX_DM_DATA = 0; // Dependency: DotMatrix
	EX_DM_SEL = 0; // Dependency: DotMatrix
	EX_LED = 0; // Dependency: LED
}

extern void Init(void){
	init_devices();
	init_training_kit();
	lcdInit(); // Dependency: LCD
	_delay_ms(1000);
	lcdClear(); // Dependency: LCD
	_delay_ms(1000);
}
