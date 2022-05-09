/* 
 * Authors: Ayberk Gokmen - 2380442
 *          Muhammed Tayyip Öztürk - 2380806
 *          Nilufer Tak - 2310506
 *          Muhammed Yakup Demirtas - 2380285
 *
 */
#pragma config OSC = HSPLL
#include <xc.h>
#include <stdint.h>
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
uint8_t is_lose = 0;
uint8_t is_end = 0;
uint8_t inp_port_btn_st = 0;
uint8_t game_start_flag = 0;
uint8_t note_count = 0;
uint8_t shift_count;
uint8_t portg_0_inp = 0; //portg press testing
uint8_t portg_1_inp = 0;
uint8_t portg_2_inp = 0;
uint8_t portg_3_inp = 0;
uint8_t portg_4_inp = 0;
uint8_t first_game = 1;
uint8_t _7seg[4]; //array for seven segment display
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
    shift_count = 11;
    note_count = 0;
    level = 1; //initial level is 1
    health_point = 9; //initial health point is 9
    ADCON0bits.ADON = 0; //disable ad conv
    ADCON1 = 0x0f;
    TRISA = 0xe0; //led 0 to led 4 used for note visualization
    TRISB = 0xe0; //led 0 to led 4 used for note visualization
    TRISC = 0x01; //RC0 will be game starter port, thus we will set it as input in the beginning
    TRISD = 0xe0; //led 0 to led 4 used for note visualization
    TRISE = 0xe0; //led 0 to led 4 used for note visualization
    TRISF = 0xe0; //led 0 to led 4 used for note visualization
    TRISG = 0b011111; //PORTG 0-4 is input for matching notes
    TRISH = 0x00;
    TRISJ = 0x00;
    LATA = 0;
    LATB = 0;
    LATC = 0;
    LATD = 0;
    LATE = 0;
    LATF = 0;
    portg_0_inp = 0; //portg press testing
    portg_1_inp = 0;
    portg_2_inp = 0;
    portg_3_inp = 0;
    portg_4_inp = 0;
}
void init_irq(){
    INTCONbits.TMR0IE = 1;
    INTCONbits.GIE = 1;
    T1CON = 0b10000101; //enable timer1
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
        T0CONbits.TMR0ON = 0; // close timer
        tmr_state = TMR_DONE; //delay achived
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
void endSevenSegment(){
    _7seg[0]=13;
    _7seg[1]=14;
    _7seg[2]=15;
    _7seg[3] = -1;
}
unsigned char sendSevenSegment(unsigned char x)
{
    switch(x)
    {
        case -1: return 0x00;
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
        case 10: return 0b00111000; //L
        case 11: return 0b00111111; //O
        case 12: return 0b01101101; //S
        case 13: return 0b01111001; //E
        case 14: return 0b01010100; //n
        case 15: return 0b01011110; //d
    }
    return 0;   
}
void sevenSegmentDisplay(){
    uint8_t digit=0b00000001;
    for(i=0;i<4;i++){ 
        PORTH = digit; //select only D0 by using PORTH
        PORTJ=sendSevenSegment(_7seg[i]); //write the byte necessary to turn on segments to PORTJ
        //wait for a while 
        _delay(10000);
        PORTH = 0; //clear
        digit<<=1; // do the same for D1, D2 and D3
    }
}
void loseSevenSegment(){
    _7seg[0]=10;
    _7seg[1]=11;
    _7seg[2]=12;
    _7seg[3]=13;
}
void input_task() {      // User pushes the button.
    if (PORTGbits.RG0){
        portg_0_inp = 1;   //set the flag when pressing
    }
    else if(portg_0_inp  && PORTFbits.RF0){ //check if led is open, when releasing the button
        portg_0_inp = 0;
        PORTFbits.RF0 = 0;
    }
    else if(portg_0_inp){
        portg_0_inp = 0;
        health_point--;
    }
    
    
    if (PORTGbits.RG1){
        portg_1_inp = 1;   //set the flag when pressing
    }
    else if(portg_1_inp  && PORTFbits.RF1){ //check if led is open, when releasing the button
        portg_1_inp = 0;
        PORTFbits.RF1 = 0;
    }
    else if(portg_1_inp){ //unmatched press
        portg_1_inp = 0;
        health_point--;
    }
    
    
    if (PORTGbits.RG2){
        portg_2_inp = 1;   //set the flag when pressing
    }
    else if(portg_2_inp  && PORTFbits.RF2){ //check if led is open, when releasing the button
        portg_2_inp = 0;
        PORTFbits.RF2 = 0;
    }
    else if(portg_2_inp){ //unmatched press
        portg_2_inp = 0; 
        health_point--;
    }
    
    
    if (PORTGbits.RG3){
        portg_3_inp = 1;   //set the flag when pressing
    }
    else if(portg_3_inp  && PORTFbits.RF3){ //check if led is open, when releasing the button
        portg_3_inp = 0;
        PORTFbits.RF3 = 0;
    }
    else if(portg_3_inp){ //unmatched press
        portg_3_inp = 0;
        health_point--;
    }
    
    
    if (PORTGbits.RG4){
        portg_4_inp = 1;   //set the flag when pressing
    }
    else if(portg_4_inp  && PORTFbits.RF4){ //check if led is open, when releasing the button
        portg_4_inp = 0;
        PORTFbits.RF4 = 0;
    }
    else if(portg_4_inp){ //unmatched press
        portg_4_inp = 0;
        health_point--;
    }
    _7seg[0] = health_point; //update health point
    if(health_point <= 0){ //lose condition
        is_lose = 1;
    }
}

void timer_task() {
    switch (tmr_state) {
        case TMR_IDLE:
            if (tmr_startreq) {
                // If a start request has been issued, go to the RUN state
                tmr_startreq = 0;
                tmr(); //set timer ticks for this level
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
void generate_note_task() { // Generates random notes. It will be called every timer0 interrupt.
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
            temp <<= 13;
            current_tmr1_cnt >>= 3;
            current_tmr1_cnt |= temp; 
            break;
        case 3:
            temp = current_tmr1_cnt;
            temp <<= 11;
            current_tmr1_cnt >>= 5;
            current_tmr1_cnt |= temp; 
            break;
    }
}
void note_shift(){
    if(LATF > 0){ //if there is a unmatch note in latf, lose health point
        health_point--;
        _7seg[0] = health_point;
        if(health_point == 0){ //lose condition
            is_lose = 1;
        }
    }
    LATF = LATE;
    LATE = LATD;
    LATD = LATC;
    LATC = LATB;
    LATB = LATA;
    LATA = 0x00;
}
void led_task(){
    if(tmr_state == TMR_DONE){ //if timer is done, generate a note
        note_shift(); 
        shift_count--;
        if(note_count < (level*5)){ //check generated note count
            note_count++;
            
            generate_note_task(); //generation_note holds port number after this instruction
            switch(generation_note){
                case 0:
                    LATA = 0b00000001;
                    break;
                case 1:
                    LATA = 0b00000010;
                    break;
                case 2:
                    LATA = 0b00000100;
                    break;
                case 3:
                    LATA = 0b00001000;
                    break;
                case 4:
                    LATA = 0b00010000;
                    break;
            }
        }
        else{ //level is finished
            if(shift_count == 0){
                level++;
                _7seg[3] = level;
                if(level < 4){
                    shift_count = level*5 + 6; //reset the shift count
                    note_count = 0;
                }
                else{
                    is_end = 1;
                }
            }
            
        }
        tmr_state = TMR_IDLE;
        tmr_startreq = 1;
    }
}
void main(void) {
    mmain:
        init_ports();
        tmr(); //level is always 1 here
        init_irq();
        while(1){ //rc0 check
            if(!is_end && !is_lose && first_game){
                _7seg[0]=health_point; //initial seven segment display for level
                _7seg[1] = -1;
                _7seg[2] = -1;
                _7seg[3]=level; //initial seven segment display for health point
            }
            if(is_end){
                endSevenSegment();
                is_end = 0;
            }
            if(is_lose){
                loseSevenSegment();
                is_lose = 0;
            }
            sevenSegmentDisplay(); // to avoid flickerring
            start_input_task(); //checks rc0 repeatedly
            if(game_start_flag){ //if rc0 is pressed and released, go to main loop
                first_game = 0;
                is_lose = 0;
                is_end = 0;
                game_start_flag = 0;
                TRISC = 0x00; //PORTC leds are set as output to use in game
                timer1_task();
                tmr_startreq = 1;
                _7seg[0]=health_point; //initial seven segment display for level
                _7seg[1] = -1;
                _7seg[2] = -1;
                _7seg[3]=level; //initial seven segment display for health point
                tmr_state = TMR_IDLE;
                break;
            }
        }
        while(1){ //main loop
            sevenSegmentDisplay(); // to avoid flickerring
            input_task();
            led_task();
            if(is_lose){
                goto mmain;
            }
            if(is_end){
                goto mmain;
            }
            timer_task();
        }
}

