#include "common.h"

/**********************************************************************
 * ----------------------- GLOBAL VARIABLES ---------------------------
 **********************************************************************/


/**********************************************************************
 * ----------------------- LOCAL FUNCTIONS ----------------------------
 **********************************************************************/

char announcement[30] = "Start your homework EARLY!:)\n";

TASK(TASK0) 
{
    PIE1bits.RC1IE = 1;	// enable USART receive interrupt
	SetRelAlarm(ALARM_TSK0, 100, 5000);
	while(1) {
        WaitEvent(ALARM_EVENT);
        ClearEvent(ALARM_EVENT);
        //transmitCharAndHello('!'); //sends hello!
	}
	TerminateTask();
}


/* End of File : tsk_task0.c */