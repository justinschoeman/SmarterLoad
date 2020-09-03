#include <avr/wdt.h>

#define BUZZER_PIN 8

#define SUNSYNK_POLL_MS 1000UL

#define POWER_MAX 100
#define POWER_LOCKOUT (30UL*1000UL)

// shared state
int16_t sun_inv_power;
int16_t sun_load_power;
uint8_t sun_soc;

#include "sunsynk.h"
#include "geyser_tx.h"

unsigned long sunsynk_poll_ts;

uint8_t sys_state;
unsigned long sys_ts;

void do_state(void) {
  // process sys state
  if(sun_load_power > POWER_MAX || sun_inv_power > POWER_MAX) {
    // exceed limit
    Serial.println("POWER TOO HIGH - SHUT OFF LOAD");
    tx_set_state(0); // set shutoff state
    sys_ts = millis(); // ts keeps track of the last time the power was excessive
  } else {
    // power OK - wait for lockout time before clearing
    if(tx_state) return; // already on? no worries
    if((millis() - sys_ts) < POWER_LOCKOUT) {
      Serial.println("LOCKOUT");
      return;
    }
    Serial.println("LOCKOUT OVER - TURN ON POWER");
    tx_set_state(1);
  }
}

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

  // set up local timer
  sunsynk_poll_ts = millis();

  // set up local state
  sys_state = tx_state; // default to on
  sys_ts = millis();

  // keep watchdog ticking
  wdt_reset();
}

void loop()
{
  if((millis() - sunsynk_poll_ts) >= SUNSYNK_POLL_MS) {
    sunsynk_poll_ts = millis();
    if(!sunsynk_read()) {
      Serial.println("sunsynk read failed");
      return;
    }
    // dump stats
    Serial.print("Inverter Power: ");
    Serial.println(sun_inv_power);
    Serial.print("Load Power: ");
    Serial.println(sun_load_power);
    Serial.print("SOC: ");
    Serial.println(sun_soc);
    // process next state (safe to do this only on a read... extra 1s latency wont hurt on release)
    do_state();
    // keep watchdog ticking on every successfull read
    wdt_reset();
  }
  tx_run();
}
