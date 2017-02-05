#include "CommonHeader.h"

#define EX_STEPPING (*(volatile unsigned char*)0x8009)

#define STEP_PHASE_MAX 3
#define STEP_PHASE_MIN 0

static unsigned char step_phase[4] = {0x01, 0x02, 0x04, 0x08};

extern void MotorDriveOneTick(char cw){	
	static int step = 0;
	if(cw){
		if(step < STEP_PHASE_MAX) {
			step++;
		} else {
			step = STEP_PHASE_MIN;
		} 
	}else {
		if(step > STEP_PHASE_MIN) {
			step--;
		} else {
			step = STEP_PHASE_MAX;
		} 
	}
	EX_STEPPING = step_phase[step];
	_delay_ms(2);
}
