/*
 *  This is a very basic receiver module.  It is designed to replace the timer board in a common 
 *  off the shelf geyser timer.
 *  
 *  I used a Hellerman Tyton unit.  It has a base board with 3.3v PSU + 30A relay.  The PSU is only
 *  good for around 100uA continuous, so low current is critical.
 *  
 *  Hardware is basically a NRF24L01 module hooked up as per the library description.
 *  
 *  Software turns off radio and powers down CPU for 4s, then wakes up, puts radio in receive mode,
 *  and waits another 15ms for any packets to be received.
 *  
 *  So the transmitter must send continuously for 6s to be sure the receiver gets it.
 */
/*
CE    -> 9
CSN   -> 10 (Hardware SPI SS)
MOSI  -> 11 (Hardware SPI MOSI)
MISO  -> 12 (Hardware SPI MISO)
SCK   -> 13 (Hardware SPI SCK)
*/

//#define DEBUG

#include <SPI.h>
// https://github.com/dparson55/NRFLite.git
#include <NRFLite.h>
// https://github.com/rocketscream/Low-Power.git
#include <LowPower.h>

#define POWER_PIN 3
#define RADIO_ID 'R'
#define RADIO_CE 9
#define RADIO_CSN 10

NRFLite radio;
int rxcnt;
char rxbuf[32];
int rxlen;
int rxfail;
  
void setup()
{
  // initialise power pin and make sure it is ON
  pinMode(POWER_PIN, OUTPUT);
  digitalWrite(POWER_PIN, 1);

#ifdef DEBUG
  Serial.begin(9600);
  Serial.println("start");
  Serial.flush();
#endif

  // initialise radio
  // By default, 'init' configures the radio to use a 2MBPS bitrate on channel 100 (channels 0-125 are valid).
  while(!radio.init(RADIO_ID, RADIO_CE, RADIO_CSN)) {
#ifdef DEBUG
    Serial.println("Cannot communicate with radio");
    Serial.flush();
#endif
    // not much we can do but keep trying...
    // sleep 8s and try agsin
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  }
  // reset failure count
  rxfail = 0;
#ifdef DEBUG
  Serial.println("radio init");
  Serial.flush();
#endif
  // will read straight away - no need to init it to power down...
  //radio.powerDown();
}

void loop()
{
  // rx window
  // we loop in here after sleep...
  // use one hasData to wake the radio, sleep a little longer for packet to arrive, then read it
  
  // hasData wakes rx... (ignore response - may be a partial packet in buffer from end of last process...)
  //if(!radio.hasData()) {
  //  // give it some time
  //  // in practice, start up delay is generous enough - but let's allow 5 expansive frame windows...
  //  // ok, allow some more so we can use a power-down sleep...
  //  #ifdef DEBUG
  //  Serial.flush();
  //  #endif
  //  LowPower.powerDown(SLEEP_15MS, ADC_OFF, BOD_OFF);
  //}
  radio.hasData();
#ifdef DEBUG
  Serial.flush();
#endif
  LowPower.powerDown(SLEEP_15MS, ADC_OFF, BOD_OFF);
  // read what we got in the window
  // just read everything but only respond to the last command...
  rxcnt = 0;
  while((rxlen=radio.hasData())) {
    // must read always to clear pipe
    radio.readData(rxbuf); // Note how '&' must be placed in front of the variable name.
    if(rxlen >= 32 || rxlen <= 0) {
      // sanity check
#ifdef DEBUG
      Serial.print("Invalid packet length: ");
      Serial.println(rxlen);
#endif
      continue;
    }
    rxbuf[rxlen] = 0;
#ifdef DEBUG
    Serial.print("Read (");
    Serial.print(rxlen);
    Serial.print("): ");
    Serial.println(rxbuf);
#endif
    rxcnt++;
  }
  // radio not needed anymore? shut it down asap
  radio.powerDown();
  if(rxcnt) {
#ifdef DEBUG
    Serial.print("Process command: ");
    Serial.println(rxbuf);
#endif
    // simple command handlers
    if(strcmp(rxbuf, "POWERON") == 0) digitalWrite(POWER_PIN, 1);
    if(strcmp(rxbuf, "POWEROFF") == 0) digitalWrite(POWER_PIN, 0);
    // reset fail count
    rxfail = 0;
  } else {
#ifdef DEBUG
    Serial.println("READ FAILED!");
#endif
    rxfail++;
    // transmitter should update us every 5 minutes...
    // wait about 30% longer and then turn off the load.
    if(rxfail >= 100) {
#ifdef DEBUG
      Serial.println("TOO MANY READ FAILURES - FAIL TO ON CONDITION");
#endif
      digitalWrite(POWER_PIN, 1);
    }
  }
#ifdef DEBUG
  // flush serial before powerdown, or serial buffer will be lost/corrupted
  Serial.flush();
#endif
  // shut down CPU for 4s
  LowPower.powerDown(SLEEP_4S, ADC_OFF, BOD_OFF);
}
