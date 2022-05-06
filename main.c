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
uint8_t level;
uint8_t tick_counter; //counter at 100 gives 500ms, 80 gives 400ms, 60 gives 300ms
uint8_t timer_done = 0;
uint8_t trial = 0;
uint8_t inp_port_btn_st = 0;
uint8_t game_start_flag = 0;
unsigned char _7seg[4];
uint8_t i=0;
void tmr_isr();
typedef enum {TMR_IDLE, TMR_RUN, TMR_DONE} tmr_state_t;
tmr_state_t tmr_state = TMR_IDLE;   // Current timer state
uint8_t tmr_startreq = 0;           // Flag to request the timer to start

void __interrupt(high_priority) highPriorityISR(void) {
    if (INTCONbits.TMR0IF) tmr_isr();
}
void __interrupt(low_priority) lowPriorityISR(void) {}

void init_ports(){
    level = 1; //initial level is 1
    TRISA = 0xe0; //led 0 to led 4 used for note visualization
    TRISB = 0xe0; //led 0 to led 4 used for note visualization
    TRISC = 0x01; //RC0 will be game starter port, thus we will set it as input in the beginning
    TRISD = 0xe0; //led 0 to led 4 used for note visualization
    TRISE = 0xe0; //led 0 to led 4 used for note visualization
    TRISF = 0xe0; //led 0 to led 4 used for note visualization
    TRISG = 0x1f; //PORTG 0-4 is input for matching notes
}
void init_irq(){
    INTCONbits.TMR0IE = 1;
    INTCONbits.GIE = 1;
}
void tmr_start(uint8_t ticks) {
    tick_counter = ticks; //set the desired number of ticks
    tmr_startreq = 1; //set the start request
    tmr_state = TMR_IDLE; //go to idle state
}
// This function aborts the current timer run and goes back to IDLE
void tmr_abort() {
    T0CON &= 0x7f; // Unset TMR0ON
    tmr_startreq = 0;
    tmr_state = TMR_IDLE;
}
void tmr_preload(){
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
    tmr_preload();
}
void tmr_isr(){
    INTCONbits.TMR0IF = 0;
    if(--tick_counter == 0){
        timer_done = 1; //delay achived
    }
    else{   
        tmr_preload();
    }
}
void start_input_task() {
    if (PORTCbits.RC0){
        inp_port_btn_st = 1;
    }
    else if (inp_port_btn_st == 1) {
        // A high pulse has been observed on the PORT input
        inp_port_btn_st = 0;
        game_start_flag = 1; //RC0 is pressed, we start the game.
    }
}
unsigned char sendSevenSegment(unsigned char x)
{
    switch(x)
    {
        case 0: return 0b00111111;  // number 0
        case 1: return 0b00000110;  // number 1
        case 2: return 0b01011011;  // number 2
        case 3: return 0b01001111;  // number 3
        case 4: return 0b01100110;  // number 4
        case 5: return 0b01101101;  // number 5
        case 6: return 0b01111101;  // number 6
        case 7: return 0b00000111;  // number 7
        case 8: return 0b01111111;  // number 8
        case 9: return 0b01101111;  // number 9
        case '-': return 1<<6;      // dash (J6-g)
    }
    return 0;   
}
void sevenSegmentUpdate(){
    uint8_t digit=1;
    for(i=0;i<4;i++){ 

        PORTH |= digit&0b00001111; //select only D0 by using PORTH
        PORTJ=sendSevenSegment(_7seg[3-i]); //write the byte necessary to turn on segments to PORTJ
        //wait for a while , ne kadar bekleyecegimizi belirtmemis ??
        PORTH &= 0b11110000; //clear
        digit<<=1; // do the same for D1, D2 and D3


    }
}
void clearSevenSegment(){

    for(i=0;i<4;i++)
        _7seg[i]='-';//set all segments to dash
}
void timer_task() {
    switch (tmr_state) {
        case TMR_IDLE:
            if (tmr_startreq) {
                // If a start request has been issued, go to the RUN state
                tmr_startreq = 0;
                tmr_preload();
                INTCONbits.T0IF = 0;
                T0CON |= 0x80; // Set TMR0ON
                tmr_state = TMR_RUN;
            }
            break;
        case TMR_RUN:
            // Timer remains in the RUN state until the counter reaches its max
            // "ticks" number of times.
            break;
        case TMR_DONE:
            // State waits here until tmr_start() or tmr_abort() is called
            break;
    }
}
void main(void) {
    init_ports();
    tmr(); //level is always 1 here
    init_irq();
    while(1){ //rc0 check
        start_input_task(); //checks rc0 repeatedly
        if(game_start_flag){ //if rc0 is pressed and released, go to main loop
            TRISC = 0xe0; //PORTC leds are set as output to use in game
            break; //TODO: we may set timer flag here since game will be started immediately...
        }
    }
    while(1){ //main loop
        timer_task();
        trial++;
    }
}

