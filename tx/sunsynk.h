#include <ModbusMaster.h>

// transmit enable (active high)
#define INV_DE 2
// receive enable (active low)
#define INV_NRE 3

#define INV_ADDR 1

ModbusMaster node;

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

void setup()
{
  pinMode(8, OUTPUT);
  digitalWrite(8, 1);
  delay(500UL);
  digitalWrite(8,0);

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

void loop()
{
  uint8_t result;
  uint16_t r;
  int i;

  Serial.println("STARTING!");
  for(i = 0; i < 1024; i++) {
    //node.begin(i, Serial);
    Serial.flush();
    result = node.readHoldingRegisters(i, 1);
    Serial.println("");
    Serial.print(i);
    Serial.print(": ");
    if (result == node.ku8MBSuccess)
    {
      r = node.getResponseBuffer(0x00);
      Serial.print(r, HEX);
      Serial.print(" ");
      Serial.print(r >> 8);
      Serial.print(" ");
      Serial.print(r & 0xff);
      Serial.print(" ");
      Serial.print(r);
      Serial.println(" ");
    } else {
      Serial.println("read failed!");
    }
  }
  delay(1000);
}
