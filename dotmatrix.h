#include "CommonHeader.h"

//Extended Dot Matrix Select
//Extended Dot Matrix Data
#define EX_DM_SEL (*(volatile unsigned int *)0x8004)
#define EX_DM_DATA (*(volatile unsigned int *)0x8006)

//////////////////////////////////////////// interface //////////////////////////////////////////////

// 설명
// 사용하기 위해 초기화해야 할 것은 외부메모리 사용을 위한 MCUCR = 0x80; 한 줄 뿐이다.

// public function
extern void DotMatrixUpping(char stay);
extern void DotMatrixDowning(char stay);
extern void DotMatrixDrawDoor(int doorOpeningLevel);

//////////////////////////////////////////// --------- //////////////////////////////////////////////

static const int arrowDown[15] = { // 파싱 커서는 아래에서부터 한 칸씩 올라가야겠지, 움직일 공간 5칸 뿐이니까, for(i=5;i<=0;i--)
	0x030, 0x030, 0x030, 0x030, 0x030, 0x233, 0x1B6, 0x0FC, 0x078, 0x030, // ↓ (10라인)
	0x0, 0x0, 0x0, 0x0, 0x0 // (공백) (5라인)
};
static const int arrowUp[15] = { // 파싱 커서는 위에서부터 한 칸씩 내려와야겠지, 움직일 공간 5칸 뿐이니까, for(i=0;i<=5;i++)
	0x0, 0x0, 0x0, 0x0, 0x0, // (공백) (5라인)
	0x030, 0x078, 0x0FC, 0x1B6, 0x233, 0x030, 0x030, 0x030, 0x030, 0x030 // ↑ (10라인)
};
static const int doorFromCloseToOpen[6][10] = {
   {0x3FF, 0x030, 0x030, 0x030, 0x030, 0x030, 0x030, 0x030, 0x030, 0x3FF},
   {0x3cf, 0x048, 0x048, 0x048, 0x048, 0x048, 0x048, 0x048, 0x048, 0x3CF},
   {0x387, 0x084, 0x084, 0x084, 0x084, 0x084, 0x084, 0x084, 0x084, 0x387},
   {0x303, 0x102, 0x102, 0x102, 0x102, 0x102, 0x102, 0x102, 0x102, 0x303},
   {0x201, 0x201, 0x201, 0x201, 0x201, 0x201, 0x201, 0x201, 0x201, 0x201},
   {0,0,0,0,0,0,0,0,0,0} // wide open
};

static void DotMatrixParse(int dataParsed[], const int dataRaw[], const int indexCut, const int parseLen){
	int i;
	for(i=0;i<parseLen;i++){
		dataParsed[i] = dataRaw[indexCut + i];
	}
}

// 위에서 아래로 출력한다
// 인자로 넘어가는 데이터 10개는 각각 도트매트릭스의 한 행을 의미한다
static void DotMatrixDisplay(const int data[10]) {
	int i;
	static int currLine = 0; // 현재 행 선택
	for(i=0;i<10;i++){ // 10회 : 10줄 : 10행 : 한 화면 : 1주기
		//(한 줄 출력 시작) 
		EX_DM_SEL = (1<<currLine); // 행 선택
		EX_DM_DATA = data[currLine++]; // 그 행에 데이터를 출력
		if(currLine>9) currLine=0; // 끝까지 출력되었으면 행 넘김
		//(한 줄 출력 끝)
		_delay_ms(1); // 세븐세그먼트랑 다르게 선명하게 나온다
	}
}

extern void DotMatrixDrawDoor(int doorOpeningLevel) { // 열린 정도가 인자다
	int i;
	for(i=0;i<4;i++){
		DotMatrixDisplay(doorFromCloseToOpen[doorOpeningLevel]);
	}
}

// 한 함수마다 한 칸씩이다.
// stay가 참이면 스크롤을 한 턴 멈춘다. 
extern void DotMatrixUpping(char stay){
	static char cursor = 0;

	int i;
	int dataParsed[10];
	DotMatrixParse(dataParsed, arrowUp, cursor, 10); // 커서 기준으로 10개 끊어 담는다
	for(i=0;i<4;i++){
		DotMatrixDisplay(dataParsed); // 10밀리세컨드 주기 화면 표현을 횟수만큼 실행하고 다음으로 넘어간다
	}

	if(stay != OFF){
		if(cursor < 5){
			cursor++;
		} else {
			cursor = 0;
		}
	}
}

extern void DotMatrixDowning(char stay){
	static char cursor = 5;
	
	int i;
	int dataParsed[10];
	DotMatrixParse(dataParsed, arrowDown, cursor, 10);
	for(i=0;i<4;i++){
		DotMatrixDisplay(dataParsed);
	}

	if(stay != OFF){
		if(cursor > 0){
			cursor--;
		} else {
			cursor = 5;
		}
	}
}




