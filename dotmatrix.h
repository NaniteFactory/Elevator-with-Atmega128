#include "CommonHeader.h"

//Extended Dot Matrix Select
//Extended Dot Matrix Data
#define EX_DM_SEL (*(volatile unsigned int *)0x8004)
#define EX_DM_DATA (*(volatile unsigned int *)0x8006)

//////////////////////////////////////////// interface //////////////////////////////////////////////

// ����
// ����ϱ� ���� �ʱ�ȭ�ؾ� �� ���� �ܺθ޸� ����� ���� MCUCR = 0x80; �� �� ���̴�.

// public function
extern void DotMatrixUpping(char stay);
extern void DotMatrixDowning(char stay);
extern void DotMatrixDrawDoor(int doorOpeningLevel);

//////////////////////////////////////////// --------- //////////////////////////////////////////////

static const int arrowDown[15] = { // �Ľ� Ŀ���� �Ʒ��������� �� ĭ�� �ö󰡾߰���, ������ ���� 5ĭ ���̴ϱ�, for(i=5;i<=0;i--)
	0x030, 0x030, 0x030, 0x030, 0x030, 0x233, 0x1B6, 0x0FC, 0x078, 0x030, // �� (10����)
	0x0, 0x0, 0x0, 0x0, 0x0 // (����) (5����)
};
static const int arrowUp[15] = { // �Ľ� Ŀ���� ���������� �� ĭ�� �����;߰���, ������ ���� 5ĭ ���̴ϱ�, for(i=0;i<=5;i++)
	0x0, 0x0, 0x0, 0x0, 0x0, // (����) (5����)
	0x030, 0x078, 0x0FC, 0x1B6, 0x233, 0x030, 0x030, 0x030, 0x030, 0x030 // �� (10����)
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

// ������ �Ʒ��� ����Ѵ�
// ���ڷ� �Ѿ�� ������ 10���� ���� ��Ʈ��Ʈ������ �� ���� �ǹ��Ѵ�
static void DotMatrixDisplay(const int data[10]) {
	int i;
	static int currLine = 0; // ���� �� ����
	for(i=0;i<10;i++){ // 10ȸ : 10�� : 10�� : �� ȭ�� : 1�ֱ�
		//(�� �� ��� ����) 
		EX_DM_SEL = (1<<currLine); // �� ����
		EX_DM_DATA = data[currLine++]; // �� �࿡ �����͸� ���
		if(currLine>9) currLine=0; // ������ ��µǾ����� �� �ѱ�
		//(�� �� ��� ��)
		_delay_ms(1); // ���켼�׸�Ʈ�� �ٸ��� �����ϰ� ���´�
	}
}

extern void DotMatrixDrawDoor(int doorOpeningLevel) { // ���� ������ ���ڴ�
	int i;
	for(i=0;i<4;i++){
		DotMatrixDisplay(doorFromCloseToOpen[doorOpeningLevel]);
	}
}

// �� �Լ����� �� ĭ���̴�.
// stay�� ���̸� ��ũ���� �� �� �����. 
extern void DotMatrixUpping(char stay){
	static char cursor = 0;

	int i;
	int dataParsed[10];
	DotMatrixParse(dataParsed, arrowUp, cursor, 10); // Ŀ�� �������� 10�� ���� ��´�
	for(i=0;i<4;i++){
		DotMatrixDisplay(dataParsed); // 10�и������� �ֱ� ȭ�� ǥ���� Ƚ����ŭ �����ϰ� �������� �Ѿ��
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




