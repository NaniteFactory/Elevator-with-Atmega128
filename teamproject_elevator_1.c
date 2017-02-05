#include "ElevatorInit.h"

// #define FLOOR_MIN 0
// #define FLOOR_MAX 5

// static char destFloor = 0b00000;
#define DestFloorIsOn(floor) ((destFloor >> floor) & 0x01) // �����ݷ��� ���� ���� ���ǽĿ��� ���ڴٴ� �ǹ̴�.
#define DestFloorIsNone (destFloor == 0) // �����ݷ��� ���� ���� ���ǽĿ��� ���ڴٴ� �ǹ̴�.
#define DestFloorSet(floor, set) destFloor &= ( 0xFF ^ (0x01 << floor) ); destFloor |= (set << floor);
#define DestFloorToggle(floor) destFloor ^= (0x01 << floor);

// static char doorOpen = CLOSE;
#define OPEN 1
#define CLOSE 0

// ���� �߰��� �����
#define DOOR_OPEN_TIME_INITIAL 15
#define DOOR_OPEN_TIME_MIN 0
//
#define UP_DOWN_ANIM_DLY 50 // ȭ��ǥ�� �����̴� �ӵ��� ���� ����
//
#define DOOR_OPENING_LEVEL_MAX 5
#define DOOR_OPENING_LEVEL_MIN 0
//
#define STEP_TICK_FOR_SINGLE_FLOOR 660 // 1�������� 1���� + �� ���� = 300ƽ
#define MOTOR_SPEED 5 // �Դٰ��� �ϸ鼭 ���͸� ��� �� ��� �ϴµ� �� ���� �ּ��� ���Ͱ� �� ���� �� ƽ �� ���ΰ� // step += �� ���� TEP_TICK_FOR_SINGLE_FLOOR�� ���� �Ŵϱ�, ������ �������� ���� ���ϴ� ���� ���� ���̴�
//
#define MESSAGE_TIME_WAIT 15 // ä�͸� ���� �ð�
#define MESSAGE_TIME_MIN 0
//
#define DOOR_DELAY_MAX 4 // ���� ������ ���� �� �� ĭ ���ŵǱ���� �����ð�
#define DOOR_DELAY_MIN 0
//
#define DURATION_FORCE_OPEN 20 // ���� �� �� ������ �� ���ӽð�

// static char currDirection = NEUTRAL;
typedef enum {UP = 0, NEUTRAL = 1, DOWN = 2} Direction;

// ������������ ��Ȳ��ȭ ���� ����
static char destFloor = 0b000000; // �����ʿ������� 0, 1, 2, 3, 4, 5��
static char currFloor = 1;
static Direction currDirection = NEUTRAL;
static char doorOpen = CLOSE; // ���� ���� ���� ���³�, ���� ���� ���¿��� ���� ���ݾ� �������� �ϰų� �������� ���� �ٽ� �� ���� �ְ� �ϴ� ����
static bool doorIsInitialOpening = false; // ���� �������� �� ��ó�� �ݵ�� ���� �����߸� �ϴ�, �߰��� �ݱ� ������ ���� �� ���� ����
static char doorOpeningLevel = 0; // 0~5 ����~Ȱ¦���� 
static char doorOpenTimeLimit = DOOR_OPEN_TIME_MIN; // ���� Ÿ�̸� ����. 1�� ���δ� // �� ó������ ���� �ִ�
static bool isMoving = false;
volatile static volatile volatile bool volatile holdingForceOpen = false;
// �̻󿡼� ����� ���� ���� ���� ��

static void LcdFloorUpdate(char _destFloor, unsigned char _x, unsigned char _y){
	unsigned int presentingNum = 0;
	presentingNum += 00000 * ((_destFloor & 0x01)?1:0);
	presentingNum += 10000 * ((_destFloor & 0x02)?1:0);
	presentingNum +=  2000 * ((_destFloor & 0x04)?1:0);
	presentingNum +=   300 * ((_destFloor & 0x08)?1:0);
	presentingNum +=    40 * ((_destFloor & 0x10)?1:0);
	presentingNum +=     5 * ((_destFloor & 0x20)?1:0);
	lcd_gotoxy(_x, _y);
	lcd_putn6(presentingNum);
	// ���ʽ�: LED
	EX_LED = destFloor;
}

static bool IsWaitingForElevator(Direction direction){ // OnStartMove() ����
	int i;
	if(direction == UP){
		for(i=currFloor+1; i<6; i++){
			if(DestFloorIsOn(i)){
				return true;
			}
		}
		return false;
	} else if (direction == DOWN) {
		for(i=currFloor-1; i>0; i--){
			if(DestFloorIsOn(i)){
				return true;
			}
		}
		return false;
	}
	return false;
}

static Direction GetOppositeDirection(Direction direction){ // OnStartMove() ����
	switch(direction){
		case UP:
			return DOWN;
		case DOWN:
			return UP;
		case NEUTRAL:
			return NEUTRAL;
	}
	return direction;
}

static void OnStartMove(void){ // Runnable1()�� ������ OnFloorClick()��, Runnable2()�� ���� ������ �������� ȣ��ȴ�.
	if(currDirection == NEUTRAL){ // �� �Լ��� ȣ��Ǿ��ٸ� �̹� ������ ������ �ڴ�. �� ���� ���� ���� �� �Լ��� ȣ���ϴ� ���� �����̴�.
		return;
	}
	if(IsWaitingForElevator(currDirection)){ // ������ ����
	} else if (IsWaitingForElevator(GetOppositeDirection(currDirection))){ // ������ �����ϱ� �ݴ�� ���� �Ѵ�
		currDirection = GetOppositeDirection(currDirection);	
	} else {
		currDirection = NEUTRAL; // �׳� ���� �ִ�
		return;
	}
	isMoving = true; // ���͸� �����̴� ��尡 �ȴ�
}

static void OnFloorClick(char floor){ // Runnable1()�� ����
	// �� �ٲ�
	if(floor != currFloor){
		DestFloorToggle(floor);
	}
	LcdFloorUpdate(destFloor, 1, 2);
	// ���� ���� �Ǵ�
	if(DestFloorIsOn(floor) == ON){ // ���� ���� ������ Ȯ���Ǿ�����, �� ��� �ϳ��� �������� �����Ǿ����ϱ�, �Ʒ��� ����
		if(currDirection == NEUTRAL){ // ó������ ���� ��ư�̸�
			if (floor == currFloor) { // ���� �� ���� (���� ���Ѵ�)
			} else if (floor > currFloor) {
				currDirection = UP;
			} else /* if (floor < currFloor) */ {
				currDirection = DOWN;
			}
			OnStartMove(); // �� �������� �����δ�
		}
		// OnStartMove(); // �� �������� �����δ�
	}
}

static void DisplayStuff(void){ // Runnable0()�� ����
	static int i = 0;

	SevenSegmentDisplayFloor(currFloor);
	if(doorOpen || doorIsInitialOpening || currDirection == NEUTRAL){ // Ȱ¦ ���� ���� / ������ ���� / ���� �� ��
		DotMatrixDrawDoor(doorOpeningLevel);
		lcd_puts(1,"NEUTRAL"); 
	} else if (currDirection == UP){
		if(i == UP_DOWN_ANIM_DLY){ // ȭ��ǥ�� �����̴� �ӵ� ����
			DotMatrixUpping(OFF);
			lcd_puts(1,"UP     "); 
		} else {
			DotMatrixUpping(ON);
		}
	} else if (currDirection == DOWN) {
		if(i == UP_DOWN_ANIM_DLY){
			DotMatrixDowning(OFF);
			lcd_puts(1,"DOWN   "); 
		} else {
			DotMatrixDowning(ON);
		}
	}

	i++;
	if(i>UP_DOWN_ANIM_DLY){
		i = 0;
	}
}

static void Runnable0(void){
	DisplayStuff();
}

static void Runnable1(void){ // Ű����ġ �Է� �ޱ�
	static int i = 0;
	static char swCnt1 = MESSAGE_TIME_WAIT;
	static char swCnt2 = MESSAGE_TIME_WAIT;
	static char swCnt3 = MESSAGE_TIME_WAIT;
	static char swCnt4 = MESSAGE_TIME_WAIT;
	static char swCnt5 = MESSAGE_TIME_WAIT;

	char message = 0;
	message = PINB;

	if(i < DURATION_FORCE_OPEN) {
		i++;
	} else {
		holdingForceOpen = false;
		i = 0;
	}

	// �� ������ �ϴϱ� ���� �Լ��� ����ؼ� �������� �ʰ� �÷��׸� ���Ѵ�
	// ���� ��� �� ���� ���� �ϸ� ���Ѵ�: 0x02 �Է¿� ���� ��ȸ�� ����� ä�͸� ���� while(PINB & 0x02){}
	if(message & 0x40 || message & 0x80) { // �� ���� ���� ��ư ����
	//	if(doorOpen){
		if(!isMoving){
			if(message & 0x40){ // ����
				holdingForceOpen = true;
				i = 0;
			} else if (message & 0x80) { // ����
				doorOpenTimeLimit = DOOR_OPEN_TIME_MIN;
			}
			// ���� �� �����̸� �� ���� �ݱ⸦ �����ϴ� Ÿ�̸ӿ� ���� �ӵ��� doorOpeningLevel++�� ���� ����Ѵ� ���� �ִ밪 5
			// ���� �� �����̸� doorOpenTimeLimit = 0; �ϰ� Ÿ�̸ӿ� ���� �ӵ��� doorOpeningLevel--�� ���� ����Ѵ� ���� �ּҰ� 0
		}
		// �� ���� ���� �� �ִ� ��Ȳ�� �ƴ϶�� �׳� �기��
	}

	if(swCnt1 > MESSAGE_TIME_WAIT){
		if(message & 0x01) {
			OnFloorClick(1);
			swCnt1 = MESSAGE_TIME_MIN;
		}
	} 
	if (swCnt2 > MESSAGE_TIME_WAIT){
		if(message & 0x02) {
			OnFloorClick(2);
			swCnt2 = MESSAGE_TIME_MIN;
		}
	}
	if (swCnt3 > MESSAGE_TIME_WAIT){
		if(message & 0x04) {
			OnFloorClick(3);
			swCnt3 = MESSAGE_TIME_MIN;
		}
	}
	if (swCnt4 > MESSAGE_TIME_WAIT){
		if(message & 0x08) {
			OnFloorClick(4);
			swCnt4 = MESSAGE_TIME_MIN;
		}
	}
	if (swCnt5 > MESSAGE_TIME_WAIT){
		if(message & 0x10) {
			OnFloorClick(5);
			swCnt5 = MESSAGE_TIME_MIN;
		}
	}

	if(swCnt1 <= MESSAGE_TIME_WAIT) swCnt1++;
	if(swCnt2 <= MESSAGE_TIME_WAIT) swCnt2++;
	if(swCnt3 <= MESSAGE_TIME_WAIT) swCnt3++;
	if(swCnt4 <= MESSAGE_TIME_WAIT) swCnt4++;
	if(swCnt5 <= MESSAGE_TIME_WAIT) swCnt5++;
}

static void OnStop(char floor){ // Runnable2() ����
	DestFloorSet(floor, OFF);
	LcdFloorUpdate(destFloor, 1, 2);
	isMoving = false;
	SoundNotice(); // �÷��׸� �ø� ���� �ٸ� ������ ó���ϰ� �� ���� ������ ��¥ ���������� ���� �͵� �ƴϰ� �׳� �̷��� �ص� �ڿ�������ϱ� �̷��� ����
	doorIsInitialOpening = true;
}

static void Runnable2(void){
	static int step = 0;
	static int doorDelay = 0;
	int i = 0;
	if(isMoving){ // ���� ������ ���
		if(currDirection == UP){
			for(i = 0; i<MOTOR_SPEED; i++) MotorDriveOneTick(ON); // cw // 1�Լ� 1����
		} else if (currDirection == DOWN){
			for(i = 0; i<MOTOR_SPEED; i++) MotorDriveOneTick(OFF); // ccw
		}
		step += MOTOR_SPEED;
		//debug //	lcd_gotoxy(1,9); //	lcd_putn3(step); //	_delay_ms(100);
		if(step >= STEP_TICK_FOR_SINGLE_FLOOR) {
			if(currDirection == UP){
				currFloor++;
			} else if (currDirection == DOWN){
				currFloor--;
			}
			if((DestFloorIsNone && currDirection != NEUTRAL) || DestFloorIsOn(currFloor)){
				OnStop(currFloor);
			}
			step = 0;
		} 
	}else{ // �� ���� �ݱ� ��� // isMoving == false �� ��
		if(doorIsInitialOpening && doorDelay > DOOR_DELAY_MAX){
			doorOpeningLevel++;
			if(doorOpeningLevel > DOOR_OPENING_LEVEL_MAX){ // InitialOpening���κ��� ���� Ȱ¦ ���� �������� ��
				doorIsInitialOpening = false;
				doorOpeningLevel = DOOR_OPENING_LEVEL_MAX;
				doorOpen = OPEN;
				doorOpenTimeLimit = DOOR_OPEN_TIME_INITIAL;
			}
			doorDelay = DOOR_DELAY_MIN;
		// elif ������ ����: �� ���� ������ ���� ���� ���� �´�. �˻簡 �켱�Ѵ�.
		}else if((holdingForceOpen || doorOpenTimeLimit > DOOR_OPEN_TIME_MIN) && doorDelay > DOOR_DELAY_MAX){ // doorOpen == OPEN �� ���¿��� ���� ����
			if(doorOpeningLevel < DOOR_OPENING_LEVEL_MAX) {
				doorOpeningLevel++; // �� ������ ���� ����
			}
			if(doorOpenTimeLimit > DOOR_OPEN_TIME_MIN){
				doorOpenTimeLimit--; // ���� ���� �ð� ����
			}
			doorDelay = DOOR_DELAY_MIN;
		}else if(doorOpenTimeLimit == DOOR_OPEN_TIME_MIN  && doorDelay > DOOR_DELAY_MAX){ // ���� ����
			if(doorOpeningLevel > DOOR_OPENING_LEVEL_MIN) { // just in case
				doorOpeningLevel--;
			}
			if(doorOpeningLevel <= DOOR_OPENING_LEVEL_MIN){ // �Ǵ��� Ȯ���� ���� �ּ� ���� �� // �� �� ����? // �̰� �־�� ���� �ڿ� ���� ������ ������
				doorOpen = CLOSE;
				doorOpeningLevel = DOOR_OPENING_LEVEL_MIN;
				OnStartMove(); //////////////////////////////////////////////////
			}
			doorDelay = DOOR_DELAY_MIN;
		}

		if(doorDelay <= DOOR_DELAY_MAX) doorDelay++; 
	}
	
}

// ���� ������ UART�� �ٱ����� ������ ��ư�� ����	

///////////////////////////////////////////////////////////////

// ����

// Runnable0() : DM, SS ��� : ������ �÷��״�� DM, SS ��¸� �Ѵ�
// ȭ��ǥ�� �� ĭ�� �ö󰡴� �ӵ��� UP_DOWN_ANIM_DLY ����� ����

// Runnable1() : Ű����ġ �Է� ���� : ���� ��ư�� ���� �÷��׸� �����Ѵ�
// ä�͸� ���� ���ð��� MESSAGE_TIME_WAIT ����� ����
// DURATION_FORCE_OPEN ����� �� ������ �� �� ������ �ּ��� �����Ǵ� �ð��� ����

// Runnable2() : ���� or ����� : ���͸� �۵���Ű�ų�, ���Ͱ� �۵����� �ƴ� ���� �÷��׸� �����ؼ� ���� ���� �ݴ´�
// ���� �ӵ��� MOTOR_SPEED ����� ���� (STEP_TICK_FOR_SINGLE_FLOOR�� ������ ���� ������ ������ �� ��)
// ���� �ӵ��� DOOR_DELAY_MAX ����� ����
// �� ���� �ö󰡱���� ȸ��(ƽ)�� ���� STEP_TICK_FOR_SINGLE_FLOOR ����� ����

extern int main(void) {
	int i = 0;
	Init();
	
	LcdFloorUpdate(destFloor, 1, 2);
	nosound();

	for(;;){
		for(i=0;i<200000000;i++){
			PORTD ^= 0xff;
			switch(i%10){	
				case 1:
					Runnable1();	// �Է�	
				case 3:
				case 5:
				case 7:
					Runnable0();	// ���÷���
					break;
				case 0:
				case 2:
				case 4:
				case 6:
				case 8:
					Runnable2(); 	// ���� or ����� 
					break;
				case 9:
					Runnable1();	// �Է�	
					break;
			}//switch
		} //for
	}//for(;;)
	return 0;
}





