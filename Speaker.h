#include "CommonHeader.h"

#define MaxLine 2
#define MaxCol 16
//
#define DELAYTIME	1
#define DELAYTIME1	0

static volatile long T1HIGHCNT = 0xFD, T1LOWCNT = 0X66;
static volatile int SoundState = ON;
static volatile int Soundonoff = ON;

//////////////////////////////////////////// interface //////////////////////////////////////////////

// ����
// ����ϱ� ���� �ʱ�ȭ�ؾ� �� ��
// - DDRG�� 0x10 (5��° ��Ʈ) �Ѽ� PORTG.4 �� ����Ŀ ������� ����ϱ�
// (PORTG�� 0x10�� ���� �״� �ϴ� �ֱ�� ���� �߻��Ѵ�)
// - TIMSK�� 0x04 (3��° ��Ʈ) �Ѽ� Ÿ�̸� �����÷ο� ���ͷ�Ʈ �㰡�ϱ�
// - extern void timer1_init(void) ȣ���ؼ� ���ֺ�� ���ļ� �����ϱ�

// public data
// soundNote() �Լ��� ���ڷ� �ѱ��
#define C1		523		// ��
#define C1_		554
#define D1		587		// ��
#define D1_		622
#define E1		659		// ��
#define F1		699		// ��
#define F1_		740
#define G1		784		// ��
#define G1_		831
#define A1		880		// ��
#define A1_		932
#define B1		988		// ��
// ��Ÿ��1
#define C2		C1*2	// ��
#define C2_		C1_*2   
#define D2		D1*2	// ��
#define D2_		D1_*2  
#define E2		E1*2	// ��
#define F2		F1*2	// ��
#define F2_		F1_*2  
#define G2		G1*2	// ��
#define G2_ 	G1_*2   
#define A2		A2*2	// ��
#define A2_		A2_*2  
#define B2 		B2*2	// ��
// ��Ÿ��2
#define DLY_1	DLY_4*4	// ���� (���)
#define DLY_2	DLY_4*2	// 2����
#define DLY_4 	800		// 4���� (�߰�)
#define DLY_8	DLY_4/2	// 8����
#define DLY_16	DLY_8/2	// 16���� (ª��)
// ����

// public function
extern void timer1_init(void);
extern void soundNote(int tone, int dly);
extern void schoolsong(void);
extern void pong(void);
extern void SoundNotice(void);

// private function
ISR(TIMER1_OVF_vect);
static void delay_us(unsigned int time_us);
static void delay_ms(unsigned int timer_ms);
static void sound(int freq);
static void nosound(void);

//////////////////////////////////////////// --------- //////////////////////////////////////////////


ISR(TIMER1_OVF_vect) { //TIMER1 has overflowed
	TCNT1H = T1HIGHCNT;
	TCNT1L = T1LOWCNT;
	if(Soundonoff == ON){
		PORTG = PORTG ^ 0x10;
	}
}

static void delay_us(unsigned int time_us){
	register unsigned int i;
	for(i=0; i<time_us; i++){
		asm volatile(" PUSH R0 ");
		asm volatile(" POP R0 ");
		asm volatile(" PUSH R0 ");
		asm volatile(" POP R0 ");
		asm volatile(" PUSH R0 ");
		asm volatile(" POP R0 ");
	}
}
static void delay_ms(unsigned int timer_ms){
	unsigned int i;
	for(i=0; i<timer_ms; i++){
		delay_us(1000);
	}
}
///////////////////////////////////////////////////////////
static void sound(int freq){
	Soundonoff = ON;
	T1HIGHCNT = (0xFFFF-floor(1000000/freq)) / 0x100;
	T1LOWCNT = 0xFFFF-floor(1000000/freq) - 0xFF00;
}
static void nosound(void){
	Soundonoff = OFF;
	delay_ms(100);
}
///////////////////////////////////////////////////////////
extern void timer1_init(void) {
	TCCR1B = 0x00;
	//
	TCNT1H = T1HIGHCNT;
	TCNT1L = T1LOWCNT;
	//
	OCR1AH = 0x02;
	OCR1AL = 0x9A;
	//
	OCR1BH = 0x02;
	OCR1BL = 0x9A;
	//	
	OCR1CH = 0x02;
	OCR1CL = 0x9A;
	//
	ICR1H = 0x02;
	ICR1L = 0x9A;
	//
	TCCR1A = 0x00;
	TCCR1B = 0x02;
}
///////////////////////////////////////////////////////////
extern void soundNote(int tone, int dly){
	sound(tone);
	delay_ms(dly*2);
	nosound();
}
extern void schoolsong(void){
	soundNote(G1, DLY_4);
	soundNote(G1, DLY_4);
	soundNote(A1, DLY_4);
	soundNote(A1, DLY_4);
	soundNote(G1, DLY_4);
	soundNote(G1, DLY_4);
	soundNote(E1, DLY_2);
	//�б����̶�����
	soundNote(G1, DLY_4);
	soundNote(G1, DLY_4);
	soundNote(E1, DLY_4);
	soundNote(E1, DLY_4);
	soundNote(D1, DLY_1);
	//�������
	soundNote(G1, DLY_4);
	soundNote(G1, DLY_4);
	soundNote(A1, DLY_4);
	soundNote(A1, DLY_4);
	soundNote(G1, DLY_4);
	soundNote(G1, DLY_4);
	soundNote(E1, DLY_2);
	//�������̿츮��
	soundNote(G1, DLY_4);
	soundNote(E1, DLY_4);
	soundNote(D1, DLY_4);
	soundNote(E1, DLY_4);
	soundNote(C1, DLY_1);
	//��ٸ��Ŵ�
}
extern void pong(void){
	soundNote(C1,DLY_4);
	soundNote(D1,DLY_4);
	soundNote(E1,DLY_4);
	soundNote(E1,DLY_4);
	soundNote(C1,DLY_4);
	soundNote(E1,DLY_4);
	soundNote(G1,DLY_8);
	soundNote(A1,DLY_8);
	soundNote(G1,DLY_4);
	// �������絹��������
	soundNote(C1,DLY_4);
	soundNote(D1,DLY_4);
	soundNote(E1,DLY_4);
	soundNote(E1,DLY_4);
	soundNote(C1,DLY_4);
	soundNote(E1,DLY_4);
	soundNote(G1,DLY_8);
	soundNote(A1,DLY_8);
	soundNote(G1,DLY_4);
	// ������������������
	soundNote(A1,DLY_2);
	soundNote(G1,DLY_4);
	soundNote(E1,DLY_4);
	soundNote(A1,DLY_2);
	soundNote(G1,DLY_4);
	soundNote(E1,DLY_4);
	soundNote(D1,DLY_4);
	soundNote(D1,DLY_4);
	soundNote(C1,DLY_4);
	soundNote(D1,DLY_4);
	soundNote(E1,DLY_4);
	soundNote(G1,DLY_4);
	soundNote(G1,DLY_4);
	// ������������ָ��ָ������� 
	soundNote(A1,DLY_4);
	soundNote(A1,DLY_8);
	soundNote(G1,DLY_4);
	soundNote(A1,DLY_4);
	soundNote(C2,DLY_4);
	soundNote(C2,DLY_4);
	soundNote(C2,DLY_4);
	soundNote(G1,DLY_4);
	soundNote(G1,DLY_4);
	soundNote(E1,DLY_4);
	soundNote(D1,DLY_4);
	soundNote(C1,DLY_2);
	// �ǳ����ɾƼ��������Ĵ�
	soundNote(D1,DLY_4);
	soundNote(E1,DLY_8);
	soundNote(C1,DLY_4);
	soundNote(E1,DLY_4);
	soundNote(G1,DLY_4);
	soundNote(G1,DLY_4);
	soundNote(G1,DLY_4);
	soundNote(D1,DLY_4);
	soundNote(E1,DLY_8);
	soundNote(F1,DLY_8);
	soundNote(E1,DLY_4);
	soundNote(D1,DLY_4);
	soundNote(C1,DLY_2);
	// �츮�����յ����������־��
}
extern void SoundNotice(void){
	soundNote(C1,DLY_16);
	soundNote(D1,DLY_16);
	soundNote(G1,DLY_16);
}
