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

uint8_t tx_state; // 0 = off, 1 = on
uint8_t tx_acks;
uint8_t tx_nacks;

// set up new tx state
void tx_set_state(uint8_t newstate) {
  if(newstate == tx_state) return;
  tx_state = newstate;
  tx_acks = 0;
  tx_nacks = 0;
}

void tx_setup()
{
    if(!radio.init(RADIO_ID, RADIO_CE, RADIO_CSN)) {
        Serial.println("Cannot communicate with radio");
        while (1); // Wait here until watchdog reboots us
    }
    Serial.println("radio init");
    // receiver defaults to on, so we do too
    tx_state = 1;
    // pretend we are in ack phase
    tx_acks = TX_MIN_ACK + 1;
    tx_nacks = 0;
}

int tx_send_hard(const char * data) {
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

char sbuf[32];
int cpos = 0;

void docmd (void) {
    Serial.print("Sending: '");
    Serial.print(sbuf);
    Serial.print("':");
    if(tx_send_hard(sbuf)) {
        Serial.println("...Success");
    } else {
        Serial.println("...Failed");
    }
}

void tx() {
    while(Serial.available()) {
      if(cpos >= 32) {
        Serial.println("overflow");
        cpos = 0;
      }
      sbuf[cpos] = Serial.read();
      if(sbuf[cpos] == '\n') {
        sbuf[cpos] = 0;
        cpos = 0;
        docmd();
        continue;
      }
      cpos++;
    }
}
