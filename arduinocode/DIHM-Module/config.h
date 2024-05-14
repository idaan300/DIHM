#ifndef _CONFIG_H
#define _CONFIG_H

#include <RadioLib.h>

// how often to send an uplink - consider legal & FUP constraints - see notes
const uint32_t uplinkIntervalSeconds = 5UL * 60UL;    // minutes x seconds

#define RADIOLIB_LORAWAN_JOIN_EUI  0x0000000000000000

// the Device EUI & two keys can be generated on the TTN console 
#ifndef RADIOLIB_LORAWAN_DEV_EUI   // Replace with your Device EUI
#define RADIOLIB_LORAWAN_DEV_EUI   0x70B3D57ED0066206//0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x06, 0x62, 0x06
#endif
#ifndef RADIOLIB_LORAWAN_APP_KEY   // Replace with your App Key 
#define RADIOLIB_LORAWAN_APP_KEY   0x41, 0x83, 0xD0, 0x00, 0x89, 0x03, 0x1C, 0x6D, 0xAD, 0x89, 0x03, 0x47, 0x5E, 0xE9, 0x1D, 0xC2
#endif
#ifndef RADIOLIB_LORAWAN_NWK_KEY   // Put your Nwk Key here
#define RADIOLIB_LORAWAN_NWK_KEY   0x2C, 0xF6, 0x5C, 0x3D, 0x99, 0xCA, 0xF6, 0x21, 0x25, 0x37, 0x53, 0x2D, 0x79, 0x77, 0xBC, 0x8E
                                   //2C F6 5C 3D 99 CA F6 21 25 37 53 2D 79 77 BC 8E
#endif

const LoRaWANBand_t Region = EU868;
const uint8_t subBand = 0;  


#if defined(ARDUINO_SAMD_FEATHER_M0)
    #pragma message ("Adafruit Feather M0 with RFM95")
    #pragma message ("Link required on board")

// Heltec
#else
  #pragma message ("Unknown board - no automagic pinmap available")
  // SX1262  pin order: Module(NSS/CS, DIO1, RESET, BUSY);
  SX1262 radio = new Module(8, 14, 12, 13);

#endif

// copy over the EUI's & keys in to the something that will not compile if incorrectly formatted
uint64_t joinEUI =   RADIOLIB_LORAWAN_JOIN_EUI;
uint64_t devEUI  =   RADIOLIB_LORAWAN_DEV_EUI;
uint8_t appKey[] = { RADIOLIB_LORAWAN_APP_KEY };
uint8_t nwkKey[] = { RADIOLIB_LORAWAN_NWK_KEY };

// create the LoRaWAN node
LoRaWANNode node(&radio, &Region, subBand);


#endif
