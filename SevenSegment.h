#include "CommonHeader.h"

//Extended Seven Segment Data : output - data - number
//Extended Seven Segment Select : output - digit
#define EX_SS_DATA 	 (*(volatile unsigned char *)0x8002)
#define EX_SS_SEL	 (*(volatile unsigned char *)0x8003)

// characterSetForDisplaying[]의 대괄호 안에 요소 번호로서 넣는다
#define DOT 10
#define SPACE 11
#define H 12
#define a 13
#define p 14
#define y 15
#define o 16
#define d 17
#define b 18
#define F 19

static const unsigned char digitOn[4] = {0b11110111, 0b11111011, 0b11111101, 0b11111110}; // 맨왼쪽 3, 맨오른쪽 0
static const unsigned char characterSetForDisplaying[20] = {
	0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x27, 0x7f, 0x6f, // 0~9 숫자
	0x80, 0x00, 0x76, 0x5f, 0x73, 0x6e, 0x5c, 0x5e, 0x7c, 0x71 //DOT SPACE HapyodbF 10부터 19까지
};

//////////////////////////////////////////// interface //////////////////////////////////////////////

// 설명
// 사용하기 위해 초기화해야 할 것은 외부메모리 사용을 위한 MCUCR = 0x80; 한 줄 뿐이다.

extern void SevenSegmentDisplayFloor(char floor);

//////////////////////////////////////////// --------- //////////////////////////////////////////////

static void SevenSegmentDisplay(unsigned char n3, unsigned char n2, unsigned char n1, unsigned char n0, int nDigit){
	int i = 0;
	unsigned char data[4] = {n0,n1,n2,n3}; // data to be shown
	
	for(i = 0; i<nDigit; i++){
	 		EX_SS_SEL = digitOn[i];
	  		EX_SS_DATA = data[i];
			_delay_ms(5);
	}//for
}//display()

extern void SevenSegmentDisplayFloor(char floor){
	int i;
	for(i=0;i<4;i++){
		SevenSegmentDisplay(
			0, 0, 
			characterSetForDisplaying[(int)floor], 
			characterSetForDisplaying[F],
			2
		);
	}
} 
