#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "HT_st7735.h"
#include "config.h"
#include "Arduino.h"
HT_st7735 st7735;
#include "FS.h"
#include <LittleFS.h>
#define FORMAT_LITTLEFS_IF_FAILED true

BLEServer* pServer = NULL;
BLECharacteristic* pCharText = NULL;
BLECharacteristic* pCharRequest = NULL;
BLEDescriptor *pDescr;
BLE2902 *pBLE2902;

bool deviceConnected = false;
bool dataSent = false;
bool oldDeviceConnected = false;

const char *firstuplinkMessage = "59";
uint8_t downlinkData[64];
uint8_t prevDownlink[64];
uint8_t storedData[64];
String savedChain;
int mydata;
int totalMessages;

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
    void byteArrayToUnsignedCharArray(uint8_t *data, unsigned char *result, size_t len) {
    for (size_t i = 0; i < len; i++) {
        result[i] = static_cast<unsigned char>(data[i]);
    }
  }

  void sendData(String inp){ 
    int len = inp.length();
    int chunkSize = 20;
    USBSerial.println(inp); 
    USBSerial.println("sending data over ble"); 
      for (int i = 0; i < len; i += chunkSize) {
      String chunk = inp.substring(i, min(i + chunkSize, len));
      pCharText->setValue(chunk.c_str());
      pCharText->notify();
      // Delay to avoid congestion in the Bluetooth stack
      delay(100);
     }
//     char charArray[inp.length() + 1]; // +1 for the null terminator
//     inp.toCharArray(charArray, inp.length() + 1);
//     writeFile(LittleFS, "/data.txt",charArray); //Store chain in file
}
  
    void onWrite(BLECharacteristic *pChar) override {
        USBSerial.println("Request received");
        String fullChain = "";
        st7735.st7735_fill_screen(ST7735_BLACK);
        st7735.st7735_write_str(0, 0, "Retrieving the data via LoRa, this may take a while...", Font_11x18, ST7735_RED, ST7735_BLACK);
        size_t downlinkSize = 0;
        int attempts = 0;
        const int maxAttempts = 5; 
        while(attempts < maxAttempts) { //Get the total amount of messages
        int state = node.sendReceive(firstuplinkMessage, 1, downlinkData, &downlinkSize, true);
        //CONVERT DATA TO INT
        size_t datalength = sizeof(downlinkData) / sizeof(downlinkSize); 
        unsigned char totalUnsigned[datalength];
        byteArrayToUnsignedCharArray(downlinkData, totalUnsigned, datalength);
        String totlen = String((char*)totalUnsigned);
        USBSerial.println(totlen);
        totalMessages = totlen.toInt();
        memcpy(prevDownlink, downlinkData, downlinkSize); //Store amount as prevDownlink
        if(-1 < totalMessages < 255){break;} //If correct data received go to next step
        attempts++;
        USBSerial.println(totalMessages);}
        const char *uplinkMessage = "60";
        if(downlinkData[0] == 0){ //If no downlinkdata, read from file
           //String saved = "no connec"; readStringFromFile("/data.txt");
           st7735.st7735_fill_screen(ST7735_BLACK);
           st7735.st7735_write_str(0, 0, "No LoRa connection, retrieving prev data", Font_11x18, ST7735_RED, ST7735_BLACK);
           USBSerial.print("NO LORA CONNECTION POSSIBLE");
           sendData(savedChain);
           delay(1000);
        } else{
            USBSerial.println(totalMessages);
            st7735.st7735_fill_screen(ST7735_BLACK);
            for(attempts = 0; attempts < totalMessages;) { //For every message store data in fullChain
              int state = node.sendReceive(uplinkMessage, 1, downlinkData, &downlinkSize, true);
              //If data ok, increment num and get next message in line
              if(state == 0 && memcmp(prevDownlink, downlinkData, downlinkSize) != 0){ USBSerial.print(state);
              USBSerial.println("status OK");
              int num = atoi(uplinkMessage);
              num++;
              char updatedMessage[10]; // Buffer to hold the new string
              itoa(num, updatedMessage, 10); // Convert integer to string
              uplinkMessage = updatedMessage;
              st7735.st7735_write_str(0, 0, updatedMessage, Font_11x18, ST7735_RED, ST7735_BLACK);
              USBSerial.println(updatedMessage);
              USBSerial.println(uplinkMessage);
              memcpy(storedData, downlinkData, downlinkSize);
              //Convert data to String
              size_t arrayLength = sizeof(storedData) / sizeof(storedData[0]); 
              unsigned char unsignedCharArray[arrayLength];
              byteArrayToUnsignedCharArray(storedData, unsignedCharArray, arrayLength);
              USBSerial.println((char*)unsignedCharArray);
              String blockchain = String((char*)unsignedCharArray);
              fullChain += blockchain;
              memset(storedData, 0, sizeof(storedData)); //reset stored array so no elements are left behind
              attempts++;
              } else {int state = node.downlink(downlinkData, &downlinkSize); }//try to catch}
              delay(20);
            }
            st7735.st7735_fill_screen(ST7735_BLACK);
            st7735.st7735_write_str(0, 0, "Sending data  to app over BLE, please be patient.", Font_11x18, ST7735_RED, ST7735_BLACK);
            savedChain = fullChain;
            sendData(fullChain); //When all messages are received, send it over BLE to app 
          }
    } 
    
String readStringFromFile(const char* path) {
  File file = LittleFS.open(path, FILE_READ);
  if (!file) {
    USBSerial.println("Failed to open file for reading");
    return "";
  }
  String data = file.readString();
  file.close();
  return data;
}

void writeFile(fs::FS &fs, const char * path, const char * message){
    USBSerial.printf("Writing file: %s\r\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        USBSerial.println("- failed to open file for writing");
        return;
    }
    if(file.print(message)){
        USBSerial.println("- file written");
    } else {
        USBSerial.println("- write failed");
    }
    file.close();
}
};

void setup() {
  USBSerial.begin(115200);
  st7735.st7735_init();
  st7735.st7735_fill_screen(ST7735_BLACK);
  BLEDevice::init("ESP32-DIHM-MODULE");
  st7735.st7735_write_str(0, 10, "System start, initialising  Radio...", Font_11x18, ST7735_RED, ST7735_BLACK);
  int state = radio.begin();
  debug(state != RADIOLIB_ERR_NONE, F("Initalise radio failed"), state, true);
  USBSerial.println("Join ('login') to the LoRaWAN Network");
  state = node.beginOTAA(joinEUI, devEUI, nwkKey, appKey, true);
  node.setCSMA(6, 2, true); // Enable CSMA which tries to minimize packet loss by searching for a free channel before actually sending an uplink
  int attempts = 0;
  const int maxAttempts = 3; 
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

  pCharRequest->addDescriptor(new BLE2902());
  
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
  if(!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)){
      USBSerial.println("LittleFS Mount Failed");
      return;
   }
   else{
       USBSerial.println("Little FS Mounted Successfully");
   }


void loop() {
  if (deviceConnected) {
        //delay(100); // Do nothing
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
