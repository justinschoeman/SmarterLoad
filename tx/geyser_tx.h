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

const static uint8_t RADIO_ID = 'T';             // Our radio's id.
const static uint8_t DESTINATION_RADIO_ID = 'R'; // Id of the radio we will transmit to.
const static uint8_t PIN_RADIO_CE = 9;
const static uint8_t PIN_RADIO_CSN = 10;

struct RadioPacket // Any packet up to 32 bytes can be sent.
{
    uint8_t FromRadioId;
    uint32_t OnTimeMillis;
    uint32_t FailedTxCount;
};

NRFLite _radio;

void setup()
{
    Serial.begin(9600);
    
    // By default, 'init' configures the radio to use a 2MBPS bitrate on channel 100 (channels 0-125 are valid).
    // Both the RX and TX radios must have the same bitrate and channel to communicate with each other.
    // You can run the 'ChannelScanner' example to help select the best channel for your environment.
    // You can assign a different bitrate and channel as shown below.
    //   _radio.init(RADIO_ID, PIN_RADIO_CE, PIN_RADIO_CSN, NRFLite::BITRATE2MBPS, 100) // THE DEFAULT
    //   _radio.init(RADIO_ID, PIN_RADIO_CE, PIN_RADIO_CSN, NRFLite::BITRATE1MBPS, 75)
    //   _radio.init(RADIO_ID, PIN_RADIO_CE, PIN_RADIO_CSN, NRFLite::BITRATE250KBPS, 0)
    
    if (!_radio.init(RADIO_ID, PIN_RADIO_CE, PIN_RADIO_CSN))
    {
        Serial.println("Cannot communicate with radio");
        while (1); // Wait here forever.
    }
    Serial.println("radio init");
}

int sendHard(const char * data) {
  unsigned long ts = millis();
  while(1) {
    if(_radio.send(DESTINATION_RADIO_ID, (void *)data, strlen(data))) return 1;
    if((millis() - ts) > (10000UL*5)) {
      return 0;
    }
  }
}

char sbuf[32];
int cpos = 0;

void docmd (void) {
    Serial.print("Sending: '");
    Serial.print(sbuf);
    Serial.print("':");
    if (sendHard(sbuf))
    {
        Serial.println("...Success");
    }
    else
    {
        Serial.println("...Failed");
    }
}

void loop()
{
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
