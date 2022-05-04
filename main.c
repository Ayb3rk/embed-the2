/* 
 * File:   main.c
 * Author: ayber
 *
 * Created on 03 May?s 2022 Sal?, 13:23
 */
#include <xc.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
int level;
int tick_counter; //counter at 100 gives 500ms, 80 gives 400ms, 60 gives 300ms
int timer_done = 0;
int trial = 0;
void tmr_isr();

void __interrupt(high_priority) highPriorityISR(void) {
    if (INTCONbits.TMR0IF) tmr_isr();
}
void __interrupt(low_priority) lowPriorityISR(void) {}

void init_ports(){
    level = 1; //initial level is 1
    TRISA = 0xE0; //game leds are initialized
    TRISB = 0xE0; //led 0 to led 4 used for note visualization
    TRISC = 0xE0;
    TRISD = 0xE0;
    TRISE = 0xE0;
    TRISF = 0xE0;
}
void init_irq(){
    INTCONbits.TMR0IE = 1;
    INTCONbits.GIE = 1;
}
void tmr_preload(int level){
    TMR0 = 60;    //only tick counter will be changed for different levels
}
void tmr(){
    T0CON = 0x07; // internal clock with 1:256 prescaler, will be same for all levels
    switch (level){
        case 1: //level 1 = 500 ms delay
            tick_counter = 100;
            break;
        case 2: //level 2 = 400ms delay
            tick_counter = 80;
            break;
        case 3: //level 3 = 300ms delay
            tick_counter = 60;
            break;
        default:
            break;
    }
    tmr_preload(level);
}
void tmr_isr(){
    INTCONbits.TMR0IF = 0;
    if(--tick_counter == 0){
        timer_done = 1; //delay achived
    }
    else{
        tmr_preload(level);
    }
}
void main(void) {
    init_ports();
    tmr(); //level is always 1 here
    init_irq();
    while(1){
        trial++;
    }
}

