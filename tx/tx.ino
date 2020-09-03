#include <avr/wdt.h>

#define BUZZER_PIN 8

// shared state
int16_t sun_inv_power;
int16_t sun_load_power;
uint8_t sun_soc;

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
  delay(100UL);
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
  if(!sunsynk_read()) {
    Serial.println("sunsynk read failed");
    // just delay and return.  if it happens too often, watchdog will reboot us.
    delay(1000);
    return;
  }
  // dump stats
  Serial.print("Inverter Power: ");
  Serial.println(sun_inv_power);
  Serial.print("Load Power: ");
  Serial.println(sun_load_power);
  Serial.print("SOC: ");
  Serial.println(sun_soc);

  if(sun_load_power > 0) {
    tx_send_hard("POWEROFF");
  } else {
    tx_send_hard("POWERON");
    
  }
  delay(1000);
  // keep watchdog ticking
  wdt_reset();
}
