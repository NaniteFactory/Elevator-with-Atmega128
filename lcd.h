#define LCD_DATA	(*(volatile unsigned char *)0x8000)
#define LCD_CONTROL	(*(volatile unsigned char *)0x8001)

static char lcd_con;

#define LCD_ENABLE_ON	(LCD_CONTROL = (lcd_con|=0x04))
#define LCD_ENABLE_OFF	(LCD_CONTROL = (lcd_con&=0x0b))
#define LCD_R_W_ON	(LCD_CONTROL = (lcd_con|=0x02))
#define LCD_R_W_OFF	(LCD_CONTROL = (lcd_con&=0x0d))
#define LCD_RS_ON	(LCD_CONTROL = (lcd_con|=0x01))
#define LCD_RS_OFF	(LCD_CONTROL = (lcd_con&=0x0e))

//////////////////////////////////////////// interface //////////////////////////////////////////////

// 주의: 딜레이 넉넉히 넣어 주지 않으면 맛이 간다
// 실질적으로 쓰는 함수들 나열함

void lcdInit(void);
void lcdClear(void);
void lcd_gotoxy(unsigned char x, unsigned char y);

void lcd_putn1(unsigned int number);
void lcd_putn2(unsigned int number);
void lcd_putn3(unsigned int number);
void lcd_putn4(unsigned int number);
void lcd_putn5(unsigned int number);
void lcd_putn6(unsigned int number);

//////////////////////////////////////////// --------- //////////////////////////////////////////////

/* private function */
void lcdDelay(unsigned int d);
void lcdClear(void);
void lcdRegWrite(unsigned char reg);
void lcdDelayLong(unsigned int d);

/* public function. interface */
void lcdInit(void);
void lcdClear(void);
void lcd_putch(unsigned char reg);
void putcharInt(unsigned char reg);
void putcharHex(unsigned char reg);
void putString(char str[]);
void secondRow(void);
void lcd_gotoxy(unsigned char x, unsigned char y);

void lcd_puts(char lcd_l, char *s) {
   	lcd_gotoxy(1,lcd_l);
	for(;*s;){
        lcd_putch(*s);
		s++;
	}
}

void lcd_putss(char *s) {
	for(;*s;){
        lcd_putch(*s);
		s++;
	}
}

void secondRow(void) {
	unsigned char i;
	for(i=0; i<24; i++){
		lcd_putch(0);
		lcdDelay(30);
	}
}

void putString(char str[]) {
	unsigned char i=0;
	while(str[i]){
		if(i==16){
			secondRow();
		}
		lcd_putch(str[i]);
		lcdDelay(30);
		i++;
	}
}

//-사용자- 커서 위치 변경(x,y)
// ---> X
// |
// |
// v Y
void lcd_gotoxy(unsigned char x, unsigned char y) {
	switch(y) {
		case 1:
			lcdRegWrite(0x80+x-1);
			lcdDelay(30);
			break; 
		case 2 :
			lcdRegWrite(0xc0+x-1);
			lcdDelay(30);
			break;		
	}
}

void lcdInit(void) {
	unsigned char i, lcd_reg[6]={0x38, 0x0c, 0x06};
	LCD_ENABLE_OFF;
	LCD_R_W_ON;
	LCD_RS_ON;


	lcdDelay(200);
	for(i=0; i<3; i++) {
		lcdRegWrite(lcd_reg[i]);
		//lcdDelay(200);
	}
    lcdClear();
}

void lcdClear(void) {
	lcdRegWrite(0x01);
	lcdDelay(500);
}

void putcharInt(unsigned char reg) {
	unsigned char temp;
	
	if(reg>99){
		temp = reg / 100;
		lcd_putch( temp + 0x30 );
		reg = reg - temp*100;
		temp = reg / 10;
		lcd_putch( temp + 0x30 );
		reg = reg - temp*10;
		lcd_putch( reg + 0x30 );
	}else if(reg>9){
		lcd_putch(' ');
		temp = reg / 10;
		lcd_putch( temp + 0x30 );
		reg = reg - temp*10;
		lcd_putch( reg + 0x30 );
	}else{
		lcd_putch(' ');
		lcd_putch(' ');
		lcd_putch( reg + 0x30 );
	}
}

void putcharHex(unsigned char reg) {
	unsigned char temp;
	
	temp = reg;
	temp>>=4;
	if(temp<10){
		lcd_putch(temp + 0x30);
	}else{
		lcd_putch(temp + 0x57);
	}

	temp = reg & 0x0f;
	if(temp<10){
		lcd_putch(temp + 0x30);
	}else{
		lcd_putch(temp + 0x57);
	}
}

void lcd_putch(unsigned char reg) {
	LCD_R_W_OFF;
	LCD_RS_ON;
	lcdDelay(5);
	LCD_ENABLE_ON;
	LCD_DATA = reg;
	lcdDelay(10);
	LCD_ENABLE_OFF;
	lcdDelay(5);
}

void lcd_putn1(unsigned int number) {
	number%=10;
	lcd_putch(number+'0');
}

void lcd_putn2(unsigned int number) {
	number%=100;
	lcd_putch(number/10+'0');
	number%=10;
	lcd_putch(number+'0');
}

void lcd_puth2(unsigned int number) {
	number%=100;
	lcd_putch((number/16)+'0');
//	number%=10;
//	lcd_putch(hex_char[number&0x0f]);
	lcd_putch((number&0x0f)+'0');
}

//-사용자- 3자리 숫자 출력 예)012
void lcd_putn3(unsigned int number) {
	number%=1000;
	lcd_putch(number/100+'0');
	number%=100;
	lcd_putch(number/10+'0');
	number%=10;
	lcd_putch(number+'0');
}

void lcd_putn4(unsigned int number) {
	lcd_putn1(number/1000);
	lcd_putn3(number);
}

void lcd_putn5(unsigned int number) {
	lcd_putn2(number/1000);
	lcd_putn3(number);
}

//-사용자- 6자리 숫자 출력 예)001234
void lcd_putn6(unsigned int number) {
	lcd_putn3(number/1000);
	lcd_putn3(number);
}

void lcdRegWrite(unsigned char reg) {
	LCD_R_W_OFF;
	LCD_RS_OFF;
	lcdDelay(5);
	LCD_ENABLE_ON;
	LCD_DATA = reg;
	lcdDelay(10);
	LCD_ENABLE_OFF;
	lcdDelay(5);
}

void lcdDelay(unsigned int d) {
	volatile int di;
	while(d--)
    	for(di=0; di<2; di++);
}

void lcdDelayLong(unsigned int d) {
	unsigned char i, j, k;
	for(i=0; i<80; i++)
		for(j=0; j<50; j++)
			for(k=0; k<d; k++);
}



