
#include "irq_handlers.h"
#include <SPI.h>   
#include <NeoSWSerial.h>
#include <avr/wdt.h>
#include "signalgen.h"


#define SERIAL_DEBUG_ON
#define SERIAL_COMMANDS_ENABLED 
#ifdef SERIAL_DEBUG_ON
#define SERIAL_DEBUGLN swserial.println
#define SERIAL_DEBUG swserial.print
#else
#define SERIAL_DEBUGLN(...)
#define SERIAL_DEBUG(...)
#endif

NeoSWSerial swserial( 3, 2 );

#define ADDR_START 1
uint8_t addr = ADDR_START;
uint8_t i;
void(*cmd_ptr)(uint8_t);  

void raise_stat_change_irq( byte stat){
  //pinMode(TX_REQ_PIN, OUTPUT);
  spi_tx_buff = stat;
  digitalWrite(TX_REQ_PIN, HIGH);  //signal master to send us a status request and pick up the data
  delayMicroseconds(255); //need to keep the pin high for a while si that it triggers an interrupt on the raspi side
  digitalWrite(TX_REQ_PIN, LOW);
}

void handle_cmd(){
    //the command is the third byte in the buffer
    uint8_t cmd = spi_rx_buff[2];
    switch (cmd){
        case 'r':
          //reboot
          asm volatile ("jmp 0"); break;
        case 't': //SPI response test
          raise_stat_change_irq(ACK_CONN_OK);          
          break;                             
        case 'f': //generate freq
          startFreqGen(100); //todo
    } 
}

void change_dir(){

}

void handleRxChar( uint8_t c ){    
      switch (c){
        case 'd': 
          change_dir();
        case '+':
          addr++;break;     
        case '-':
          addr--;break; 
        case 't': //SPI response test
          raise_stat_change_irq('\r');
          break;                              
      }   

}



void setup() {
  
  //setup spi as slave
  pinMode(MISO,OUTPUT);   
  //pinMode(MISO,OUTPUT);   
  pinMode(DBG_PIN,OUTPUT);   
  prepare_timers();
  wdt_enable(WDTO_1S);   // Watchdog auf 1 s stellen
  SPCR |= _BV(SPE);      // Turn on SPI in Slave Mode via the SPI Control Register
    // turn on interrupts
  SPCR |= _BV(SPIE);
  startFreqGen(100);
  //SPI.attachInterrupt();
  //SPI.setClockDivider(SPI_CLOCK_DIV8);
  //SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));
  //SPI.attachInterrupt(spi_on_receive);

  //setup silvermax conn
  /*Serial.begin(57600, SERIAL_9N1);
  swserial.attachInterrupt( handleRxChar );
  swserial.begin( 38400 );
  swserial.println("RDY");
  pinMode(RS485_RXEN_PIN, OUTPUT);
  pinMode(TX_REQ_PIN, OUTPUT);
  digitalWrite(RS485_RXEN_PIN, LOW);  //enable reception
  digitalWrite(TX_REQ_PIN, LOW);  //enable reception
  */
  setState(MSGSTATE_ACCEPT_COMMAND);
  raise_stat_change_irq(ACK_RESET);
}

#define RCV_ERR_INVALID_STARTBYTE 1

void serialEvent() { //callback for ack receival
/*
   int r = Serial.read();

   //we don't check the CRC, so ignore the last byte
   rcv_bit ++;
   if (rcv_bit == 1){ //first bit is the address
      if ((r & 0x0100) != 0) 
      {
        //ss.print("ADDR ");
        //ss.println((r & 0x00FF), HEX);  
        qt_resp.rcv_addr = r & 0x00FF;  
        return;
      }else{
        qt_resp.ack_resp = ACK_ERR_INVALID_STARTBYTE;
      }
   } else if (rcv_bit == 2){
      if ( r & 0b10000000) { //if the eigth bit is set, it's an ack
        QS_RESPONSE_RECEIVED(ACK_OK);
        rcv_bit = 0;
        return;
      } else { //if the eigth bit is not set, it's a nack following the no of words
        qt_resp.no_of_words = (byte) r;
      } 
    } else if (rcv_bit == 3){ //NACK
      if ( r == 0xff) qt_resp.ack_resp = ACK_NOK;
      else qt_resp.ack_resp = ACK_OK;
      QS_RESPONSE_RECEIVED(qt_resp.ack_resp);
    }
    if (rcv_bit == (qt_resp.no_of_words + 2)){
        //qt_resp.resp_received = true;
        DISABLE_ACK_TIMER;
        rcv_bit = 0;
    }
*/    
}

uint16_t msg_cnt;
void loop() { 
  //delay(200);
  wdt_reset();

  if (iCmdErr>0){
    iCmdErr = 0;
    SERIAL_DEBUGLN("CMD TOUT");
    raise_stat_change_irq(ACK_ERR_INVALID_COMMAND);
  }
  //delay(100);
  if (bProcessCommand){
      setState(MSGSTATE_CMD_PROCESSING); //do not accept new commands before this has been processed
      bProcessCommand = false;
      if (spi_buff_pos < 2) {
        spi_tx_buff = ACK_ERR_INVALID_COMMAND;
      }else{
        //swserial.write('S');
        //swserial.println(msg_cnt++); 
        if (spi_rx_buff[0] == 0x00) {
          handle_cmd();
          spi_buff_pos = 0;
          //setState(MSGSTATE_WAIT_ACK);
        }
      }
      setState(MSGSTATE_ACCEPT_COMMAND);
  } 
  /*
  if (qt_resp.resp_received){
    SERIAL_DEBUGLN("QR"); 
    //swserial.println(qt_resp.ack_resp, HEX);
    raise_stat_change_irq(qt_resp.ack_resp);
    memset((void*) &qt_resp, 0, sizeof(qt_resp_struct));
    rcv_bit = 0;
    //qt_resp.resp_received = 0;
  }
  */
  #ifdef SERIAL_COMMANDS_ENABLED
  
  if (cmd_ptr != 0){
    SERIAL_DEBUGLN(addr); 
    cmd_ptr(addr);
    cmd_ptr = 0;
    start_await_resp_timer();
  }
  
  #endif
}
