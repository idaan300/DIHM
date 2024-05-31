#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <EEPROM.h>
#include "HT_st7735.h"
#include "config.h"
#include "Arduino.h"
HT_st7735 st7735;

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
BLECharacteristic* pCharacteristic_2 = NULL;
BLEDescriptor *pDescr;
BLE2902 *pBLE2902;
bool deviceConnected = false;
bool dataSent = false;
bool oldDeviceConnected = false;
uint32_t value = 0;
const char *uplinkMessage = "Send";
uint8_t downlinkData[64];
uint8_t storedData[64];

#define EEPROM_SIZE 128
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define CHARACTERISTIC_UUID_2 "ad237abf-fd9f-400a-b8a0-fe9da237134a"

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) override {
        deviceConnected = true;
    }
    void onDisconnect(BLEServer* pServer) override {
        deviceConnected = false;
    }
};

class CharacteristicCallback: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pChar) override {
        USBSerial.println("R received");
        st7735.st7735_fill_screen(ST7735_BLACK);
        st7735.st7735_write_str(0, 0, "Retrieving the data via LoRa, this may take a while...", Font_11x18, ST7735_RED, ST7735_BLACK);
        size_t downlinkSize = 64;
        int state = node.sendReceive(uplinkMessage, 1, downlinkData, &downlinkSize, true); //uplink and downlink same function  
        if(downlinkData[0] == 0){ USBSerial.print(downlinkData[1]); USBSerial.println("waiting for downlink"); }
        //while(downlinkData[0] == 0){int state = node.sendReceive(uplinkMessage, 1, downlinkData, &downlinkSize, true); USBSerial.print(state);USBSerial.println(downlinkData[0],HEX);delay(500);}
        int attempts = 0;
        const int maxAttempts = 10; 
        while(downlinkData[0] == 0 && attempts < maxAttempts) {
            int state = node.sendReceive(uplinkMessage, 1, downlinkData, &downlinkSize, true);
            USBSerial.print(state);
            USBSerial.println(downlinkData[0], HEX);
            delay(500);
            attempts++;
        }
        if(state != RADIOLIB_LORAWAN_NO_DOWNLINK) {
        // Did we get a downlink with data for us
        if(downlinkSize > 0) {
             USBSerial.println("status OK");
             debug((state != RADIOLIB_LORAWAN_NO_DOWNLINK) && (state != RADIOLIB_ERR_NONE), F("Error in sendReceive"), state, false);
             memcpy(storedData, downlinkData, downlinkSize);
             //saveData(downlinkData, downlinkSize);
        }
       }
    }
};

void setup() {
  USBSerial.begin(115200);
  EEPROM.begin(EEPROM_SIZE);
  st7735.st7735_init();
  st7735.st7735_fill_screen(ST7735_BLACK);
  BLEDevice::init("ESP32-DIHM-MODULE");
  st7735.st7735_write_str(0, 10, "System start, initialising  Radio...", Font_11x18, ST7735_RED, ST7735_BLACK);
  int state = radio.begin();
  debug(state != RADIOLIB_ERR_NONE, F("Initalise radio failed"), state, true);
  USBSerial.println("Join ('login') to the LoRaWAN Network");
  state = node.beginOTAA(joinEUI, devEUI, nwkKey, appKey, true);
  int attempts = 0;
  const int maxAttempts = 10; 
        while(state != 0 && attempts < maxAttempts) {
            state = node.beginOTAA(joinEUI, devEUI, nwkKey, appKey, true);
            USBSerial.println(state); delay(500);
            attempts++;
        }
  //while(state != 0){state = node.beginOTAA(joinEUI, devEUI, nwkKey, appKey, true); USBSerial.println(state); delay(500);};
  USBSerial.print("[LoRaWAN] DevAddr: ");
  USBSerial.println((unsigned long)node.getDevAddr(), HEX);
  USBSerial.println("Ready!\n");
//  if(state < RADIOLIB_ERR_NONE) {st7735.st7735_write_str(0, 0, "--failed---", Font_7x10, ST7735_RED, ST7735_BLACK);
//  USBSerial.println("Error...");}
  
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );
  pCharacteristic_2 = pService->createCharacteristic(
        CHARACTERISTIC_UUID_2,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_WRITE 
    );
  pCharacteristic_2->setCallbacks(new CharacteristicCallback());
    
  pCharacteristic->addDescriptor(new BLE2902());
  pCharacteristic_2->addDescriptor(new BLE2902());
  
  pService->start();
  dataSent = false;
  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  USBSerial.println("Waiting a client connection to notify...");
  st7735.st7735_fill_screen(ST7735_BLACK);
  st7735.st7735_write_str(0, 10, "Please connect with bluetooth", Font_11x18, ST7735_RED, ST7735_BLACK);
  // Build payload byte array
}

void saveData(const uint8_t* data, size_t dataSize) {
    EEPROM.write(0, dataSize);
    for (size_t i = 0; i < dataSize && i < EEPROM_SIZE; i++) {
        EEPROM.write(i + 1, data[i]);
    }
    EEPROM.commit();
}

void loadData(uint8_t* data, size_t& dataSize) {
    dataSize = EEPROM.read(0);
    for (size_t i = 0; i < dataSize && i < EEPROM_SIZE; i++) {
        data[i] = EEPROM.read(i + 1);
    }
}
void byteArrayToUnsignedCharArray(uint8_t *data, unsigned char *result, size_t len) {
    for (size_t i = 0; i < len; i++) {
        result[i] = static_cast<unsigned char>(data[i]);
    }
}


void loop() {
  if (deviceConnected) {
        //st7735.st7735_write_str(0, 0, "--BLE connect---", Font_7x10, ST7735_RED, ST7735_BLACK);
//        if(dataSent == false){ sendLora();
        st7735.st7735_fill_screen(ST7735_BLACK);
        st7735.st7735_write_str(0, 0, "Retrieving the data via LoRa, this may take a while...", Font_11x18, ST7735_RED, ST7735_BLACK);
//        } //Request data
        size_t arrayLength = sizeof(storedData) / sizeof(storedData[0]); 
        unsigned char unsignedCharArray[arrayLength];
        byteArrayToUnsignedCharArray(storedData, unsignedCharArray, arrayLength);
////        for (int i = 0; i < sizeof(storedData); i++) {
////          if(storedData[i] != 0){USBSerial.print(storedData[i], HEX); // Print the byte in hexadecimal format
////          USBSerial.print(" "); }// Print a space between bytes}
////      }
        String fullString = (char *)unsignedCharArray;
        //USBSerial.println(unsignedCharArray[0],HEX);
        if(unsignedCharArray[0] != 0 && dataSent == false){ sendData(unsignedCharArray);}
//        //if(convertedData!= "" | storedData[0] != 0){ sendData(unsignedCharArray); }// sendData(lora_downlink);}
        delay(1000); // bluetooth stack will go into congestion, if too many packets are sent, in 6 hours test i was able to go as low as 3ms
    }

    if (!deviceConnected && oldDeviceConnected) {
        delay(500);
        pServer->startAdvertising();
        oldDeviceConnected = deviceConnected;
        dataSent = false;
    }

    if (deviceConnected && !oldDeviceConnected) {
        oldDeviceConnected = deviceConnected;
        dataSent = false;
    }
}

//void sendLora(){
////    unsigned long startTime = millis();  // Record the start time
////    while (millis() - startTime < timeOut) {
////        int state = node.downlink(downlinkData, &downlinkSize);
////        if (downlinkSize + sizeof(storedData) < sizeof(storedData)) { // Check if there is enough space
////            memcpy(storedData + downlinkSize, downlinkData, downlinkSize); // Append downlinkData to storedData
////        } else {
////            //debug(true, F("storedData is full"), false);
////            break; // Exit the loop, storedData is full
////        }
////      }
//}

void sendData(unsigned char inp[]){
    //fullstring = given bij LoRa downlink and should be decode from base64
    String fullString = (char *)inp;
    USBSerial.println(fullString); 
    USBSerial.println(dataSent); 
    int len = fullString.length();
    int chunkSize = 20;
    if(dataSent == false && len > 1){
      USBSerial.println("sending data over ble"); 
      for (int i = 0; i < len; i += chunkSize) {
    // Get the substring of the full string for this chunk
      String chunk = fullString.substring(i, min(i + chunkSize, len));
      // Set the value of the characteristic to the chunk
      USBSerial.println(chunk.c_str());
      pCharacteristic->setValue(chunk.c_str());
      // Notify the characteristic
      pCharacteristic->notify();
      // Delay to avoid congestion in the Bluetooth stack
      delay(50); // You may adjust this delay as needed
     }
     dataSent = true;
  }
    
}
