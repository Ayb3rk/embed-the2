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
unsigned char _7seg[4];
int i=0;
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
    T0CON |= 0x80; // Set TMR0ON
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
    tmr_preload(level);
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
    int digit=1;
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
void main(void) {
    init_ports();
    tmr(); //level is always 1 here
    init_irq();
    while(1){
        trial++;
    }
}

