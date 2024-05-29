#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define CHARACTERISTIC_UUID_2 "ad237abf-fd9f-400a-b8a0-fe9da237134a"

BLEServer* pServer = nullptr;
BLECharacteristic* pCharacteristic = nullptr;
BLECharacteristic* pCharacteristic_2 = nullptr;
bool deviceConnected = false;
bool oldDeviceConnected = false;
bool dataSent = false;
String serverUrl = "http://20.218.152.90/webrequest";

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
        USBSerial.println(ssidArray);
        USBSerial.println(passArray);
        WiFi.begin(ssidArray, passArray);
        USBSerial.println("WIFI CONNECTED");
    }
};

void setup() {
    USBSerial.begin(115200);
    WiFi.mode(WIFI_STA);
    USBSerial.println("Device setup");
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

void loop() {
    if (deviceConnected) {
       if (!dataSent) {
            USBSerial.println("BLE CONNECTED");
            String data = sendWifi();
            int len = data.length();
            if(len >= 1){
            int chunkSize = 20;
            for (int i = 0; i < len; i += chunkSize) {
              String chunk = data.substring(i, min(i + chunkSize, len));
              USBSerial.println(chunk);
              pCharacteristic->setValue(chunk.c_str());
              pCharacteristic->notify();
              delay(50);
            }
            dataSent = true;
        }
      }
    }

    if (!deviceConnected && oldDeviceConnected) {
        delay(500); // Give the BLE stack time to reset
        pServer->startAdvertising(); // Restart advertising
        oldDeviceConnected = deviceConnected;
        dataSent = false;
    }

    if (deviceConnected && !oldDeviceConnected) {
        oldDeviceConnected = deviceConnected;
        dataSent = false;
    }
}

String sendWifi() {
  USBSerial.println("Attempting webreq");
    if (WiFi.status() == WL_CONNECTED) {
     USBSerial.println("SUCCESS?");
     HTTPClient http;
     http.begin(serverUrl);
        int httpResponseCode = http.GET();
        if (httpResponseCode > 0) {
          String payload = http.getString();
          size_t l = 256;
          return payload;
          //USBSerial.println(payload);
          //sendData(payload);
          //stringToUint8Array(payload, storedData, l);
          USBSerial.println("HTTP Response code: " + String(httpResponseCode));
        }
        else {
            USBSerial.println("Error in HTTP request, code: " + String(httpResponseCode));
            return "error";
            }
        http.end();
    } else {
      return "";
    }
 }

//void sendData(String data) {
//    if (!dataSent) {
//        //String fullString = (char*)data;
//        int len = data.length();
//        int chunkSize = 20;
//
//        for (int i = 0; i < len; i += chunkSize) {
//            String chunk = data.substring(i, min(i + chunkSize, len));
//            USBSerial.println(chunk);
//            pCharacteristic->setValue(chunk.c_str());
//            pCharacteristic->notify();
//            delay(1000);
//        }
//        dataSent = true;
//    }
//}
