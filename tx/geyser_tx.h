/*

Demonstrates simple RX and TX operation.
Any of the Basic_RX examples can be used as a receiver.
Please read through 'NRFLite.h' for a description of all the methods available in the library.

Radio    Arduino
CE    -> 9
CSN   -> 10 (Hardware SPI SS)
MOSI  -> 11 (Hardware SPI MOSI)
MISO  -> 12 (Hardware SPI MISO)
SCK   -> 13 (Hardware SPI SCK)
IRQ   -> No connection
VCC   -> No more than 3.6 volts
GND   -> GND

*/

#include <SPI.h>
#include <NRFLite.h>

// our ID
#define RADIO_ID 'T'
// receiver ID
#define RADIO_DST_ID 'R'
#define RADIO_CE 9
#define RADIO_CSN 10

NRFLite radio;

// min acks before we consider state received
#define TX_MIN_ACK 5
// send time (make ~50% longer than the receiver poll interval
#define TX_SEND_MS 6000UL
// poll interval (once command is acked, resend every this interval...)
#define TX_POLL_MS 60000UL

uint8_t tx_state; // 0 = off, 1 = on
uint8_t tx_acks;
uint8_t tx_nacks;
unsigned long tx_ts;

// set up new tx state
void tx_set_state(uint8_t newstate) {
  if(newstate == tx_state) return;
  tx_state = newstate;
  tx_acks = 0;
  tx_nacks = 0;
  tx_ts = millis();
}

void tx_setup()
{
  if(!radio.init(RADIO_ID, RADIO_CE, RADIO_CSN)) {
    Serial.println("Cannot communicate with radio");
    while (1) {} // Wait here until watchdog reboots us
  }
  Serial.println("radio init");
  // receiver defaults to on, so we do too
  tx_state = 255; // ensure bad state
  tx_set_state(1); // set up whole state
}

int8_t tx_send_hard(const char * data) {
  unsigned long ts = millis();
  int8_t ret = 0;
  
  Serial.print("Sending: ");
  Serial.println(data);
  // long process - reset watchdog before we try
  wdt_reset();
  // tight loop - must get as many transmissions as possible in in 15ms window receiver is awake...
  while((millis() - ts) <= TX_SEND_MS) {
    if(radio.send(RADIO_DST_ID, (void *)data, strlen(data))) {
      ret = 1;
      break;
    }
  }
  // long process - reset watchdog when done
  wdt_reset();
  Serial.print("Send result: ");
  Serial.println(ret);
  return ret;
}

// perform any necessary tx actions
void tx_run(void) {
  int8_t i;
  
  if(tx_acks >= TX_MIN_ACK) {
    // we have already received the minimum number of acks - only send every X seconds
    if((millis() - tx_ts) < TX_POLL_MS) return;
    // we are due to send now...
  }
  // debug
  Serial.print("state: ");
  Serial.println(tx_state);
  Serial.print("acks: ");
  Serial.println(tx_acks);
  Serial.print("nacks: ");
  Serial.println(tx_nacks);
  if(tx_state) {
    i = tx_send_hard("POWERON");
  } else {
    i = tx_send_hard("POWEROFF");
  }
  // we are just sending for security - ignore result
  if(tx_acks >= TX_MIN_ACK) return;
  // sending to change state? track/check results
  if(i) {
    tx_acks++;
  } else {
    tx_nacks++;
    if(tx_nacks + tx_acks > 2*TX_MIN_ACK) {
      // too many nacks - force reboot
      Serial.println("Too many nacks... Forcing reboot.");
      while(1) {}
    }
  }
}
