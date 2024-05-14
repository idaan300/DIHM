#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <BLE2902.h>
#include <EEPROM.h>
#include "HT_st7735.h"
#include "Arduino.h"
#include "config.h"

#define EEPROM_SIZE 128
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define CHARACTERISTIC_UUID_2 "ad237abf-fd9f-400a-b8a0-fe9da237134a"

HT_st7735 st7735;
const char* serverUrl = "http://20.218.152.90/webrequest";

BLEServer* pServer = nullptr;
BLECharacteristic* pCharacteristic = nullptr;
BLECharacteristic* pCharacteristic_2 = nullptr;
bool deviceConnected = false;
bool oldDeviceConnected = false;
bool dataSent = false;
uint8_t downlinkData[64];
uint8_t storedData[64];
const char *uplinkMessage = "S";

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
        std::string value = pChar->getValue();
        String valueStr = String(value.c_str());
        int split = valueStr.indexOf(';');
        String ssid = valueStr.substring(0, split);
        String pass = valueStr.substring(split + 1);

        char ssidArray[32], passArray[32];
        ssid.toCharArray(ssidArray, sizeof(ssidArray));
        pass.toCharArray(passArray, sizeof(passArray));

        WiFi.begin(ssidArray, passArray);
    }
};

void setup() {
    USBSerial.begin(115200);
    EEPROM.begin(EEPROM_SIZE);
    st7735.st7735_init();
    st7735.st7735_fill_screen(ST7735_BLACK);

     int state = radio.begin();
    state = node.beginOTAA(joinEUI, devEUI, nwkKey, appKey, true);

    BLEDevice::init("ESP32-DIHM-MODULE");

    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    BLEService *pService = pServer->createService(SERVICE_UUID);
    pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_WRITE |
        BLECharacteristic::PROPERTY_NOTIFY
    );

    pCharacteristic_2 = pService->createCharacteristic(
        CHARACTERISTIC_UUID_2,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_WRITE |
        BLECharacteristic::PROPERTY_NOTIFY
    );
    pCharacteristic_2->setCallbacks(new CharacteristicCallback());

    pCharacteristic->addDescriptor(new BLE2902());
    pCharacteristic_2->addDescriptor(new BLE2902());

    pService->start();
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setMinPreferred(0x0);
    BLEDevice::startAdvertising();
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

void loop() {
    if (deviceConnected) {
        st7735.st7735_write_str(0, 0, "--BLE connect---", Font_7x10, ST7735_RED, ST7735_BLACK);
        //sendLora();
        sendWifi();
        st7735.st7735_write_str(0, 0, "--LoRa Sent---", Font_7x10, ST7735_RED, ST7735_BLACK);
        sendData();
        delay(1000);
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

void sendWifi() {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(serverUrl);
        int httpResponseCode = http.GET();

        if (httpResponseCode > 0) {
            String payload = http.getString();
            USBSerial.println("HTTP Response code: " + String(httpResponseCode));
        } else {
            USBSerial.println("Error in HTTP request, code: " + String(httpResponseCode));
        }
        http.end();
    } else {
    }
}

//void sendLora() {
//    size_t downlinkSize = 16;
//    int state = node.sendReceive(uplinkMessage, 1, downlinkData, &downlinkSize, true);
//    if (state == -1101) {
//        loadData(downlinkData, downlinkSize);
//        sendWifi();
//    } else {
//        saveData(downlinkData, downlinkSize);
//    }
//    memcpy(storedData, downlinkData, downlinkSize);
//}

void sendData() {
    if (!dataSent) {
        String fullString = (char*)storedData;
        int len = fullString.length();
        int chunkSize = 20;

        for (int i = 0; i < len; i += chunkSize) {
            String chunk = fullString.substring(i, min(i + chunkSize, len));
            pCharacteristic->setValue(chunk.c_str());
            pCharacteristic->notify();
            delay(5);
        }
        dataSent = true;
    }
}
