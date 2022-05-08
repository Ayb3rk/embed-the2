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
typedef struct {char loByte, hiByte;} byte_count;
union  {
    byte_count bytes;
    uint16_t word;
}timer_count ;
int generation_note;
uint16_t test;
uint16_t current_tmr1_cnt;
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
uint8_t health_point;

void __interrupt(high_priority) highPriorityISR(void) {
    if (INTCONbits.TMR0IF) tmr_isr();
}
void __interrupt(low_priority) lowPriorityISR(void) {}

void init_ports(){
    level = 1; //initial level is 1
    health_point = 9; //initial health point is 9
    _7seg[0]=level; //initial seven segment display for level 
    _7seg[3]=health_point; //initial seven segment display for health point
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
    T1CON = 0b11111001;
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
        case 'l': return 0b00001110;
        case 'o': return 0b01111110;
        case 's': return 0b01011011;
        case 'e': return 0b01001111;    
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
void loseSevenSegment(){
    _7seg[0]='l';
    _7seg[1]='o';
    _7seg[2]='s';
    _7seg[3]='e';
}
void timer_task() {
    switch (tmr_state) {
        case TMR_IDLE:
            if (tmr_startreq) {
                // If a start request has been issued, go to the RUN state
                tmr_startreq = 0;
                tmr_preload();
                INTCONbits.T0IF = 0;
                T0CON |= 0xc0; // Set TMR0ON
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
void timer1_task() {      // Save the Timer 1 value to generate random notes.
    timer_count.bytes.loByte = TMR1L;
    timer_count.bytes.hiByte = TMR1H;
    current_tmr1_cnt = timer_count.word;
}
void generate_node_task() { // Generates random notes. It will be called every timer0 interrupt.
    test = current_tmr1_cnt & 0b0000000000000111;
    generation_note = test % 5;
    uint16_t temp;
    switch (level) {
        case 1:
            temp = current_tmr1_cnt;
            temp = temp << 15;
            current_tmr1_cnt = current_tmr1_cnt >> 1;
            current_tmr1_cnt |= temp; 
            break;
        case 2:
            temp = current_tmr1_cnt;
            temp << 13;
            current_tmr1_cnt >> 3;
            current_tmr1_cnt |= temp; 
            break;
        case 3:
            temp = current_tmr1_cnt;
            temp << 11;
            current_tmr1_cnt >> 5;
            current_tmr1_cnt |= temp; 
            break;
    }
}
void input_task() {      // User pushes the button.
    if (PORTGbits.RG0 && PORTFbits.RF0){
        PORTFbits.RF0 = 0;   //turn off the lamb.
    }
    else if (PORTGbits.RG1 && PORTFbits.RF1){
        PORTFbits.RF1 = 0;   //turn off the lamb.
    }
    else if (PORTGbits.RG2 && PORTFbits.RF2){
        PORTFbits.RF2 = 0;   //turn off the lamb.
    }
    else if (PORTGbits.RG3 && PORTFbits.RF3){
        PORTFbits.RF3 = 0;   //turn off the lamb.
    }
    else if (PORTGbits.RG4 && PORTFbits.RF4){
        PORTFbits.RF4 = 0;   //turn off the lamb.
    }
    else if (PORTGbits.RG0 || PORTGbits.RG1 || PORTGbits.RG2 || PORTGbits.RG3 || PORTGbits.RG4) {
        --health_point; //Wrong button pressed.
    }
    else;
    
    if(!health_point){
        loseSevenSegment();
        sevenSegmentUpdate(); ////LOSE is visible in seven segment display 
        game_start_flag = 0;    //and game shoul be ended.
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
            timer1_task();
            tmr_startreq = 1;
            break; //TODO: we may set timer flag here since game will be started immediately...
        }
    }
    while(1){ //main loop
        sevenSegmentUpdate(); // to avoid flickerring
        timer_task();
        input_task();
        if(!game_start_flag)    break;    //lose situation. Game ends.
        trial++;
    }
}

