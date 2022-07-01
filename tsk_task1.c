#include "common.h"

/**********************************************************************
 * ----------------------- GLOBAL VARIABLES ---------------------------
 **********************************************************************/
extern char rcvd_chr;
/**********************************************************************
 * ----------------------- LOCAL FUNCTIONS ----------------------------
 **********************************************************************/


TASK(TASK1) 
{
    PIE1bits.RC1IE = 1;	// enable USART receive interrupt
    SetRelAlarm(ALARM_TSK1, 100, 50);
	while(1) {
        WaitEvent(ALARM_EVENT);
        ClearEvent(ALARM_EVENT);
        /*if(rcvd_chr != 0)
        {
            transmitCharAndHello(rcvd_chr); //hello+char_received
            rcvd_chr = 0;
        }*/
	}
	TerminateTask();
}

/* End of File : tsk_task1.c */