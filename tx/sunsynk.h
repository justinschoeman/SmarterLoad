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

void sunsynk_read()
{
  uint8_t result;
  uint16_t val;
  int i;

  //node.begin(i, Serial);
  Serial.flush();
  result = node.readHoldingRegisters(173, 13);
  if(result == node.ku8MBSuccess) {
    for(i = 0; i < 13; i++) {
      val = node.getResponseBuffer(i);
      Serial.print(173+i);
      Serial.print(" ");
      Serial.println(val);
    }
  } else {
    Serial.println("read failed!");
  }
}
