/*
  RadioLib LoRaWAN Starter Example

  This example joins a LoRaWAN network and will send
  uplink packets. Before you start, you will have to
  register your device at https://www.thethingsnetwork.org/
  After your device is registered, you can run this example.
  The device will join the network and start uploading data.

  Running this examples REQUIRES you to check "Resets DevNonces"
  on your LoRaWAN dashboard. Refer to the network's 
  documentation on how to do this.

  For default module settings, see the wiki page
  https://github.com/jgromes/RadioLib/wiki/Default-configuration

  For full API reference, see the GitHub Pages
  https://jgromes.github.io/RadioLib/

  For LoRaWAN details, see the wiki page
  https://github.com/jgromes/RadioLib/wiki/LoRaWAN

*/
#include "config.h"
#include "HT_st7735.h"
#include "Arduino.h"
HT_st7735 st7735;

void setup() {
  Serial.begin(115200);
  while (!Serial);
  delay(5000);  // Give time to switch to the serial monitor
  Serial.println(F("\nSetup ... "));
  st7735.st7735_init();
  st7735.st7735_fill_screen(ST7735_BLACK);
  Serial.println(F("Initalise the radio"));
  st7735.st7735_write_str(0, 0, "--init radio---", Font_7x10, ST7735_RED, ST7735_BLACK);
  int state = radio.begin();
  debug(state != RADIOLIB_ERR_NONE, F("Initalise radio failed"), state, true);
  Serial.println(F("Join ('login') to the LoRaWAN Network"));
  state = node.beginOTAA(joinEUI, devEUI, nwkKey, appKey, true);
  if(state < RADIOLIB_ERR_NONE) {st7735.st7735_write_str(0, 0, "--failed---", Font_7x10, ST7735_RED, ST7735_BLACK);}
  debug(state < RADIOLIB_ERR_NONE, F("Join failed"), state, true);

  Serial.println(F("Ready!\n"));
}


void loop() {
  st7735.st7735_fill_screen(ST7735_BLACK);
  Serial.println(F("Sending uplink"));

  // Read some inputs
  uint8_t Digital1 = digitalRead(2);
  uint16_t Analog1 = analogRead(3);

  // Build payload byte array
  uint8_t uplinkPayload[3];
  uplinkPayload[0] = Digital1;
  uplinkPayload[1] = highByte(Analog1);   // See notes for high/lowByte functions
  uplinkPayload[2] = lowByte(Analog1);
  // Perform an uplink
  int state = node.sendReceive(uplinkPayload, sizeof(uplinkPayload));    
  debug((state != RADIOLIB_LORAWAN_NO_DOWNLINK) && (state != RADIOLIB_ERR_NONE), F("Error in sendReceive"), state, false);
  Serial.print(F("Uplink complete, next in "));
  Serial.print(uplinkIntervalSeconds);
  Serial.println(F(" seconds"));
  
  // Wait until next uplink - observing legal & TTN FUP constraints
  delay(uplinkIntervalSeconds * 1000UL);  // delay needs milli-seconds
}
