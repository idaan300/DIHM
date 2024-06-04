#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <EEPROM.h>
#include "config.h"
#include "Arduino.h"
BLEServer* pServer = NULL;
BLECharacteristic* pCharText = NULL;
BLECharacteristic* pCharRequest = NULL;
BLEDescriptor *pDescr;
BLE2902 *pBLE2902;
#include "HT_st7735.h"
HT_st7735 st7735;
bool deviceConnected = false;
bool dataSent = false;
bool oldDeviceConnected = false;
const char *uplinkMessage = "Send";
uint8_t downlinkData[64];
uint8_t storedData[64];

#define EEPROM_SIZE 128
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define CHARACTERISTIC_UUID_2 "ad237abf-fd9f-400a-b8a0-fe9da237134a"


class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

class CharacteristicCallback: public BLECharacteristicCallbacks {
    void byteArrayToUnsignedCharArray(uint8_t *data, unsigned char *result, size_t len) {
    for (size_t i = 0; i < len; i++) {
        result[i] = static_cast<unsigned char>(data[i]);
    }
  }

  void sendData(unsigned char inp[]){
    String fullString = "| User: Joris, Descr: Version 1.0 of uploaded file, File: ihm_part1.pdf, Time: 09/04/2024 11:03:23 ";//String fullString = (char *)inp;
    USBSerial.println(fullString); 
    USBSerial.println(dataSent); 
    int len = fullString.length();
    int chunkSize = 10;
    USBSerial.println("sending data over ble"); 
      for (int i = 0; i < len; i += chunkSize) {
      String chunk = fullString.substring(i, min(i + chunkSize, len));
      pCharText->setValue(chunk.c_str());
      pCharText->notify();
      // Delay to avoid congestion in the Bluetooth stack
      delay(500); // You may adjust this delay as needed
     }
}

void loadData(uint8_t* data, size_t& dataSize) {
    dataSize = EEPROM.read(0);
    for (size_t i = 0; i < dataSize && i < EEPROM_SIZE; i++) {
        data[i] = EEPROM.read(i + 1);
    }
}
  
    void onWrite(BLECharacteristic *pChar) override {
        USBSerial.println("R received");
        size_t downlinkSize = 64;
        int attempts = 0;
        const int maxAttempts = 1; 
        while(downlinkData[0] == 0 && attempts < maxAttempts) {
            int state = node.sendReceive(uplinkMessage, 1, downlinkData, &downlinkSize, true);
            USBSerial.print(state);
            USBSerial.print(downlinkData[0], HEX);USBSerial.print(downlinkData[1], HEX);USBSerial.print(downlinkData[2], HEX);
            delay(500);
            attempts++;
        }
        if(attempts >= maxAttempts){
           loadData(downlinkData,downlinkSize);
           memcpy(storedData, downlinkData, downlinkSize);
           size_t arrayLength = sizeof(storedData) / sizeof(storedData[0]); 
           unsigned char unsignedCharArray[arrayLength];
           byteArrayToUnsignedCharArray(storedData, unsignedCharArray, arrayLength);
           sendData(unsignedCharArray);
           delay(1000);
        }
 
    }
};


void setup() {
  USBSerial.begin(115200);
  //EEPROM.begin(EEPROM_SIZE);
  st7735.st7735_init();
  st7735.st7735_fill_screen(ST7735_BLACK);
  BLEDevice::init("ESP32-DIHM-MODULE");
  st7735.st7735_write_str(0, 10, "System start, initialising  Radio...", Font_11x18, ST7735_RED, ST7735_BLACK);
  int state = radio.begin();
  USBSerial.println("Join ('login') to the LoRaWAN Network");
  state = node.beginOTAA(joinEUI, devEUI, nwkKey, appKey, true);
  int attempts = 0;
  const int maxAttempts = 1; 
        while(state != 0 && attempts < maxAttempts) {
            state = node.beginOTAA(joinEUI, devEUI, nwkKey, appKey, true);
            USBSerial.println(state); delay(500);
            attempts++;
        }
  USBSerial.print("[LoRaWAN] DevAddr: ");
  USBSerial.println((unsigned long)node.getDevAddr(), HEX);
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharText = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
  pCharRequest = pService->createCharacteristic(
        CHARACTERISTIC_UUID_2,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_WRITE 
    );
  pCharRequest->setCallbacks(new CharacteristicCallback());
    
  pBLE2902 = new BLE2902();
  pBLE2902->setNotifications(true);
  pCharText->addDescriptor(pBLE2902);

  //pCharRequest->addDescriptor(new BLE2902());
  
  pService->start();
  dataSent = false;
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  USBSerial.println("Waiting a client connection to notify...");
  st7735.st7735_fill_screen(ST7735_BLACK);
  st7735.st7735_write_str(0, 10, "Please connect with bluetooth", Font_11x18, ST7735_RED, ST7735_BLACK);
}

void loop() {

    String fullString = "| User: {Joris}, Descr: {Version 1.0 of uploaded file}, File: {ihm_part1.pdf}, Time: {09/04/2024 11:03:23} ";
    // notify changed value | User: {Joris}, Des
    if (deviceConnected) {
        st7735.st7735_fill_screen(ST7735_BLACK);
        st7735.st7735_write_str(0, 0, "Retrieving the data via LoRa, this may take a while...", Font_11x18, ST7735_RED, ST7735_BLACK);
        delay(1000); // bluetooth stack will go into congestion, if too many packets are sent, in 6 hours test i was able to go as low as 3ms
    }
    // disconnecting
    if (!deviceConnected && oldDeviceConnected) {
        delay(500); // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        USBSerial.println("start advertising");
        dataSent = false;
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected) {
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
        dataSent = false;
    }
}
