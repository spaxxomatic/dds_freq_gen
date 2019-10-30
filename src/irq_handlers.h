#include <stdlib.h>
#include <Arduino.h>
#include "spi_proto.h"

#define LED_PIN 13 //arduino pro mini has a led on pin 13
#define DBG_PIN 5 //arduino pro mini has a led on pin 13


#define MSGSTATE_ACCEPT_COMMAND 0
#define MSGSTATE_RECEIVING_COMMAND 1
#define MSGSTATE_STAT_REQ 2
#define MSGSTATE_CMD_RECEIVED 3
#define MSGSTATE_WAIT_ACK 4
#define MSGSTATE_ACK_TIMEOUT 5
#define MSGSTATE_LOCAL_CMD 6
#define MSGSTATE_CMD_PROCESSING 7

#define DISABLE_CMD_WAIT_TIMER bitClear(TIMSK2, TOIE2);

#define DISABLE_ACK_TIMER bitClear(TIMSK1, OCIE1A); 

extern byte spi_rx_buff[32] ;
extern byte spi_tx_buff;
extern volatile char msgstate;
extern uint8_t payload_len;
extern volatile uint8_t spi_buff_pos;
extern volatile bool bProcessCommand;
void prepare_timers(void);
void start_await_resp_timer(void);

void setState(uint8_t new_state);

//the TX_REQ_PIN connects to a master IRQ and signalizes a trasmit request
//when received, the master should initiate an SPI trasmission and pick the slave's message
//#define TX_REQ_PIN 4 //PD4  
#define TX_REQ_PIN 9 

extern volatile uint8_t awaitState ;
extern volatile uint8_t iAckTimerTick;
extern volatile uint8_t iCmdErr ;


typedef struct  {
  uint8_t rcv_addr ;
  //uint8_t rcv_error ;
  byte ack_resp ;
  byte nack_msg ;
  bool resp_received ;
  byte no_of_words ;
} qt_resp_struct ;

extern volatile qt_resp_struct qt_resp ;
extern volatile uint8_t rcv_bit;
