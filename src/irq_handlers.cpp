#include "irq_handlers.h"
#include <SPI.h>   

volatile uint8_t awaitState = false;
volatile uint8_t iAckTimerTick  = 0;
volatile uint8_t iCmdErr  = 0;

volatile uint8_t spi_buff_pos;
volatile bool bProcessCommand;
byte spi_rx_buff[32] ;
byte spi_tx_buff;

volatile char msgstate;
uint8_t payload_len;

volatile uint8_t rcv_bit;
volatile qt_resp_struct qt_resp = {0};

void setState(uint8_t new_state){
  //swserial.print("SS");
  //swserial.println(new_state);
  msgstate = new_state;
}

void prepare_timers(void){
   
   TCCR2A = 0; // set entire TCCR1A register to 0
   TCCR2B = (1<<CS22)|(1<<CS21)|(1<<CS20);
   
   TCCR1A = 0; // set entire TCCR1A register to 0
   TCCR1B = 0; // same for TCCR1B
   TCCR1B |= (1<<WGM12)|(1<<CS12)|(1<<CS10);

}


void start_await_resp_timer(void){

   //Timer1 will timeout if a response is not received in a timely manner
   //Timer Clock = 1/1024 of sys clock
   //Mode = CTC (Clear Timer On Compare)
   //OCR1A=20; //compare value: x*F_CPU/1024
   //OCR1A = 1953; //16 MHz with 1024 prescaler 15624/8=1953 -> 8 Hz interrupt frequency   
   OCR1A = 8*1953; //16 MHz with 1024 prescaler 15624/8=1953 -> 8 Hz interrupt frequency   
   TIFR1 = 1<<OCF1A; //reset the interrupt flag, which might be set and generate a spurious interrupt
   TIMSK1 |=(1<<OCIE1A);  //Output compare 1A interrupt enable
  //OCR1A = 15624; // set compare match register to desired timer count. 16 MHz with 1024 prescaler = 15624 counts/s
   sei();
}

void start_command_rcv_wd_timer(void){
   //Timer will timeout if a command is not received in a timely manner
   //Timer Clock = 1/1024 of sys clock
   //Mode = Overflow
   
   TCNT2 = 200; 
   //will count up to 255 then rise the ovfl interrupt, so we have a time of ~ TCNT2*1024/16000000 

   TIFR2 = 1<<TOV2; //reset the interrupt flag, which might be set and generate a spurious interrupt
   TIMSK2 |= 1<<TOIE2;  //Overflow interrupt enable
   //digitalWrite(DBG_PIN, 1);
   sei();
}


ISR (SPI_STC_vect)
{  
  byte c = SPDR;  // grab byte from SPI Data Register
  if (msgstate == MSGSTATE_ACCEPT_COMMAND ) { 
    //if (c == 0x00) msgstate = MSGSTATE_LOCAL_CMD; //a command addressed to us, and not to the servos
    if (c == 0xFF){
      //status request is indicated when the first byte is 0xFF
       //setState(MSGSTATE_STAT_REQ); 
       //bProcessCommand = true;
       SPDR = spi_tx_buff;
       return;
    } else {
      setState(MSGSTATE_RECEIVING_COMMAND);
      start_command_rcv_wd_timer(); //if this timer expires, the message receive state machine will be reset
    } 
    spi_buff_pos = 0; //transmission start
    payload_len = 0;
  } 
  
  if (msgstate == MSGSTATE_RECEIVING_COMMAND) {// a command comes in 
    // add to buffer if room
    if (spi_buff_pos == 1) //second byte is the length of the message payload
      payload_len = c + 2 ; //address is the first, then comes the len, so we add 2
    
    if (spi_buff_pos < (sizeof (spi_rx_buff))){
      spi_rx_buff [spi_buff_pos] = c;
      spi_buff_pos++;
      //SPDR = c ;  // send back 
    }else{
      SPDR = ACK_ERR_BUFFER_OVERFLOW ;   
    }    
    if (spi_buff_pos == payload_len) { //last byte has arrived
      DISABLE_CMD_WAIT_TIMER;
      setState(MSGSTATE_CMD_RECEIVED);
      bProcessCommand = true;
    } 
  } 
}

ISR(TIMER1_COMPA_vect) //qs response receive timeout timer
{
  DISABLE_ACK_TIMER;
  //memset((void*) &qt_resp, 0, sizeof(qt_resp_struct));
  rcv_bit = 0;
  //msgstate = MSGSTATE_ACK_TIMEOUT;
  iAckTimerTick++;
}

ISR(TIMER2_OVF_vect) //command wait timeout timer
{
  DISABLE_CMD_WAIT_TIMER;
  setState(MSGSTATE_ACCEPT_COMMAND);
  iCmdErr ++;
  //spi_buff_pos = 0; //transmission start
  //payload_len = 0;
}

