#include "ElevatorInit.h"

// #define FLOOR_MIN 0
// #define FLOOR_MAX 5

// static char destFloor = 0b00000;
#define DestFloorIsOn(floor) ((destFloor >> floor) & 0x01) // 세미콜론이 없는 것은 조건식에서 쓰겠다는 의미다.
#define DestFloorIsNone (destFloor == 0) // 세미콜론이 없는 것은 조건식에서 쓰겠다는 의미다.
#define DestFloorSet(floor, set) destFloor &= ( 0xFF ^ (0x01 << floor) ); destFloor |= (set << floor);
#define DestFloorToggle(floor) destFloor ^= (0x01 << floor);

// static char doorOpen = CLOSE;
#define OPEN 1
#define CLOSE 0

// 대충 추가한 상수들
#define DOOR_OPEN_TIME_INITIAL 15
#define DOOR_OPEN_TIME_MIN 0
//
#define UP_DOWN_ANIM_DLY 50 // 화살표가 움직이는 속도가 느린 정도
//
#define DOOR_OPENING_LEVEL_MAX 5
#define DOOR_OPENING_LEVEL_MIN 0
//
#define STEP_TICK_FOR_SINGLE_FLOOR 660 // 1개층까지 1바퀴 + 반 바퀴 = 300틱
#define MOTOR_SPEED 5 // 왔다갔다 하면서 모터를 잠깐 봐 줘야 하는데 그 때에 최소한 모터가 한 번에 몇 틱 돌 것인가 // step += 한 다음 TEP_TICK_FOR_SINGLE_FLOOR랑 비교할 거니까, 나누어 떨어지는 수로 정하는 편이 좋을 것이다
//
#define MESSAGE_TIME_WAIT 15 // 채터링 방지 시간
#define MESSAGE_TIME_MIN 0
//
#define DOOR_DELAY_MAX 4 // 문이 열리고 닫힐 때 한 칸 갱신되기까지 지연시간
#define DOOR_DELAY_MIN 0
//
#define DURATION_FORCE_OPEN 20 // 열림 한 번 눌렀을 때 지속시간

// static char currDirection = NEUTRAL;
typedef enum {UP = 0, NEUTRAL = 1, DOWN = 2} Direction;

// 엘리베이터의 상황변화 감지 변수
static char destFloor = 0b000000; // 오른쪽에서부터 0, 1, 2, 3, 4, 5층
static char currFloor = 1;
static Direction currDirection = NEUTRAL;
static char doorOpen = CLOSE; // 전부 열려 젖힌 상태나, 열려 젖힌 상태에서 문이 조금씩 닫히려고 하거나 닫히려는 도중 다시 열 수도 있고 하는 구간
static bool doorIsInitialOpening = false; // 층에 도착했을 때 맨처음 반드시 문이 열려야만 하는, 중간에 닫기 눌러서 닫을 수 없는 구간
static char doorOpeningLevel = 0; // 0~5 닫힘~활짝열림 
static char doorOpenTimeLimit = DOOR_OPEN_TIME_MIN; // 모터 타이머 기준. 1씩 줄인다 // 맨 처음에는 닫혀 있다
static bool isMoving = false;
volatile static volatile volatile bool volatile holdingForceOpen = false;
// 이상에서 상수랑 전역 변수 선언 끝

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
	// 보너스: LED
	EX_LED = destFloor;
}

static bool IsWaitingForElevator(Direction direction){ // OnStartMove() 소유
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

static Direction GetOppositeDirection(Direction direction){ // OnStartMove() 소유
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

static void OnStartMove(void){ // Runnable1()이 소유한 OnFloorClick()과, Runnable2()의 문이 닫히는 시점에서 호출된다.
	if(currDirection == NEUTRAL){ // 이 함수가 호출되었다면 이미 방향은 정해진 뒤다. 즉 방향 정한 다음 이 함수를 호출하는 것이 사용법이다.
		return;
	}
	if(IsWaitingForElevator(currDirection)){ // 방향대로 간다
	} else if (IsWaitingForElevator(GetOppositeDirection(currDirection))){ // 끝까지 왔으니까 반대로 가야 한다
		currDirection = GetOppositeDirection(currDirection);	
	} else {
		currDirection = NEUTRAL; // 그냥 멈춰 있다
		return;
	}
	isMoving = true; // 모터를 움직이는 모드가 된다
}

static void OnFloorClick(char floor){ // Runnable1()의 소유
	// 불 바뀜
	if(floor != currFloor){
		DestFloorToggle(floor);
	}
	LcdFloorUpdate(destFloor, 1, 2);
	// 진행 방향 판단
	if(DestFloorIsOn(floor) == ON){ // 누른 곳이 목적지 확정되었으면, 즉 적어도 하나의 목적지가 설정되었으니까, 아래를 실행
		if(currDirection == NEUTRAL){ // 처음으로 켜진 버튼이면
			if (floor == currFloor) { // 방향 본 다음 (방향 정한다)
			} else if (floor > currFloor) {
				currDirection = UP;
			} else /* if (floor < currFloor) */ {
				currDirection = DOWN;
			}
			OnStartMove(); // 그 방향으로 움직인다
		}
		// OnStartMove(); // 그 방향으로 움직인다
	}
}

static void DisplayStuff(void){ // Runnable0()의 소유
	static int i = 0;

	SevenSegmentDisplayFloor(currFloor);
	if(doorOpen || doorIsInitialOpening || currDirection == NEUTRAL){ // 활짝 열린 상태 / 열리는 도중 / 진행 안 함
		DotMatrixDrawDoor(doorOpeningLevel);
		lcd_puts(1,"NEUTRAL"); 
	} else if (currDirection == UP){
		if(i == UP_DOWN_ANIM_DLY){ // 화살표가 움직이는 속도 설정
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

static void Runnable1(void){ // 키스위치 입력 받기
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

	// 꾹 눌러야 하니까 다음 함수를 배려해서 폴링하지 않고 플래그만 셋한다
	// 예를 들어 이 따위 짓을 하면 망한다: 0x02 입력에 대한 공회전 방식의 채터링 방지 while(PINB & 0x02){}
	if(message & 0x40 || message & 0x80) { // 문 열림 닫힘 버튼 가정
	//	if(doorOpen){
		if(!isMoving){
			if(message & 0x40){ // 열림
				holdingForceOpen = true;
				i = 0;
			} else if (message & 0x80) { // 닫힘
				doorOpenTimeLimit = DOOR_OPEN_TIME_MIN;
			}
			// 눌린 게 열림이면 문 열고 닫기를 관장하는 타이머에 일정 속도로 doorOpeningLevel++할 것을 당부한다 변수 최대값 5
			// 눌린 게 닫힘이면 doorOpenTimeLimit = 0; 하고 타이머에 일정 속도로 doorOpeningLevel--할 것을 당부한다 변수 최소값 0
		}
		// 문 열고 닫을 수 있는 상황이 아니라면 그냥 흘린다
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

static void OnStop(char floor){ // Runnable2() 소유
	DestFloorSet(floor, OFF);
	LcdFloorUpdate(destFloor, 1, 2);
	isMoving = false;
	SoundNotice(); // 플래그만 올린 다음 다른 곳에서 처리하게 할 수도 있지만 진짜 엘레베이터 만들 것도 아니고 그냥 이렇게 해도 자연스러우니까 이렇게 하자
	doorIsInitialOpening = true;
}

static void Runnable2(void){
	static int step = 0;
	static int doorDelay = 0;
	int i = 0;
	if(isMoving){ // 모터 움직임 모드
		if(currDirection == UP){
			for(i = 0; i<MOTOR_SPEED; i++) MotorDriveOneTick(ON); // cw // 1함수 1스텝
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
	}else{ // 문 열고 닫기 모드 // isMoving == false 일 때
		if(doorIsInitialOpening && doorDelay > DOOR_DELAY_MAX){
			doorOpeningLevel++;
			if(doorOpeningLevel > DOOR_OPENING_LEVEL_MAX){ // InitialOpening으로부터 문이 활짝 열려 젖혀졌을 때
				doorIsInitialOpening = false;
				doorOpeningLevel = DOOR_OPENING_LEVEL_MAX;
				doorOpen = OPEN;
				doorOpenTimeLimit = DOOR_OPEN_TIME_INITIAL;
			}
			doorDelay = DOOR_DELAY_MIN;
		// elif 순서에 주의: 문 열림 조건은 닫힘 조건 위에 온다. 검사가 우선한다.
		}else if((holdingForceOpen || doorOpenTimeLimit > DOOR_OPEN_TIME_MIN) && doorDelay > DOOR_DELAY_MAX){ // doorOpen == OPEN 의 상태에서 열림 조건
			if(doorOpeningLevel < DOOR_OPENING_LEVEL_MAX) {
				doorOpeningLevel++; // 문 열림의 정도 설정
			}
			if(doorOpenTimeLimit > DOOR_OPEN_TIME_MIN){
				doorOpenTimeLimit--; // 열림 제한 시간 설정
			}
			doorDelay = DOOR_DELAY_MIN;
		}else if(doorOpenTimeLimit == DOOR_OPEN_TIME_MIN  && doorDelay > DOOR_DELAY_MAX){ // 닫힘 조건
			if(doorOpeningLevel > DOOR_OPENING_LEVEL_MIN) { // just in case
				doorOpeningLevel--;
			}
			if(doorOpeningLevel <= DOOR_OPENING_LEVEL_MIN){ // 되는지 확인한 다음 주석 지울 것 // 왜 안 되지? // 이게 있어야 멈춘 뒤에 다음 층으로 진행함
				doorOpen = CLOSE;
				doorOpeningLevel = DOOR_OPENING_LEVEL_MIN;
				OnStartMove(); //////////////////////////////////////////////////
			}
			doorDelay = DOOR_DELAY_MIN;
		}

		if(doorDelay <= DOOR_DELAY_MAX) doorDelay++; 
	}
	
}

// 여유 있으면 UART로 바깥에서 누르는 버튼도 구현	

///////////////////////////////////////////////////////////////

// 설명서

// Runnable0() : DM, SS 출력 : 설정된 플래그대로 DM, SS 출력만 한다
// 화살표가 한 칸씩 올라가는 속도를 UP_DOWN_ANIM_DLY 상수로 조절

// Runnable1() : 키스위치 입력 받음 : 눌린 버튼에 따라서 플래그를 설정한다
// 채터링 방지 대기시간을 MESSAGE_TIME_WAIT 상수로 조절
// DURATION_FORCE_OPEN 상수는 문 열림이 한 번 눌리면 최소한 유지되는 시간을 설정

// Runnable2() : 모터 or 도어개폐 : 모터를 작동시키거나, 모터가 작동중이 아닐 때는 플래그를 설정해서 문을 열고 닫는다
// 모터 속도는 MOTOR_SPEED 상수로 조절 (STEP_TICK_FOR_SINGLE_FLOOR를 나머지 없이 나누는 값으로 할 것)
// 개폐 속도는 DOOR_DELAY_MAX 상수로 조절
// 한 층을 올라가기까지 회전(틱)의 수를 STEP_TICK_FOR_SINGLE_FLOOR 상수로 조절

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
					Runnable1();	// 입력	
				case 3:
				case 5:
				case 7:
					Runnable0();	// 디스플레이
					break;
				case 0:
				case 2:
				case 4:
				case 6:
				case 8:
					Runnable2(); 	// 모터 or 도어개폐 
					break;
				case 9:
					Runnable1();	// 입력	
					break;
			}//switch
		} //for
	}//for(;;)
	return 0;
}





