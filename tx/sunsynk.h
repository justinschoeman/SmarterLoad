//https://github.com/4-20ma/ModbusMaster.git
#include <ModbusMaster.h>

// transmit enable (active high)
#define INV_DE 2
// receive enable (active low)
#define INV_NRE 3

#define INV_ADDR 1

ModbusMaster node;

// rs485 transceiver mode control
void preTransmission()
{
  digitalWrite(INV_NRE, 1);
  digitalWrite(INV_DE, 1);
}

void postTransmission()
{
  digitalWrite(INV_NRE, 0);
  digitalWrite(INV_DE, 0);
}

// setup
void sunsynk_setup()
{

  pinMode(INV_NRE, OUTPUT);
  pinMode(INV_DE, OUTPUT);
  // Init in receive mode
  digitalWrite(INV_NRE, 0);
  digitalWrite(INV_DE, 0);

  // Modbus communication runs at 115200 baud
  Serial.begin(9600);

  node.begin(INV_ADDR, Serial);
  // Callbacks allow us to configure the RS485 transceiver correctly
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);
}

int8_t sunsynk_read()
{
  uint8_t result;
  int16_t val;
  int i;

  // flush serial before enabling 485 transceiver!
  Serial.flush();
  result = node.readHoldingRegisters(175, 10);
  if(result == node.ku8MBSuccess) {
    /*
    for(i = 0; i < 13; i++) {
      val = node.getResponseBuffer(i);
      Serial.print(173+i);
      Serial.print(" ");
      Serial.println(val);
    }
    */
    sun_inv_power = node.getResponseBuffer(0);
    sun_load_power = node.getResponseBuffer(3);
    sun_soc = node.getResponseBuffer(9);
  } else {
    Serial.println("read failed!");
    return 0;
  }
  return 1;
}
