#include <avr/wdt.h>

#define BUZZER_PIN 8

#include "sunsynk.h"
#include "geyser_tx.h"

void setup()
{
  // start watchdog
  wdt_enable(WDTO_8S);

  // beep the beeper
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, 1);
  delay(250UL);
  digitalWrite(BUZZER_PIN, 0);
  delay(250UL);
  digitalWrite(BUZZER_PIN, 1);
  delay(250UL);
  digitalWrite(BUZZER_PIN, 0);

  // keep watchdog ticking
  wdt_reset();

  // set up serial port
  Serial.begin(9600);

  // set up display
  Serial.println("Init sunsynk");
  sunsynk_setup();

  // set up transmitter
  Serial.println("Init tx");
  tx_setup();
}

void loop()
{
  sunsynk_read();
  // keep watchdog ticking
  wdt_reset();
}
