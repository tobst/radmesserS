/*
    BLE Device pretending to be a Heart Rate Measurement Device but sending distance instead
    
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLETests/SampleNotify.cpp
    Ported to Arduino ESP32 by Evandro Copercini
    Distance Measurement by Marc Rene Friess: https://www.smarthomeng.de/entfernungsmessung-auf-basis-eines-esp32-und-smarthomeng
    
    Used BLE Scanner https://play.google.com/store/apps/details?id=com.macdom.ble.blescanner&hl=de to find out what original Heart Rate Sensor is doing and copied services and characteristics UUIDs
    
*/
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEDescriptor.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Wire.h>
#include <EEPROM.h>

// define the number of bytes to store
#define EEPROM_SIZE 1

// PINs
const int triggerPin = 15;
const int echoPin = 4;

// VARs
const int runs = 20;
unsigned long measureInterval = 1000;
unsigned long StartTime = millis();
unsigned long CurrentTime = millis();
uint8_t handleBarWidth = 0;
int timeout = 15000;

BLECharacteristic *pCharacteristic;
BLECharacteristic *pSystemIDCharcateristic;
BLECharacteristic *pModelNumberStringCharacteristic;
BLECharacteristic *pSerialNumberStringCharacteristic;
BLECharacteristic *pFirmwareRevisionCharacteristic;
BLECharacteristic *pHardwareRevisionCharacteristic;
BLECharacteristic *pSoftwareRevisionCharacteristic;
BLECharacteristic *pManufacturerNameCharacteristic;
BLECharacteristic *pSensorLocationCharacteristic;
BLECharacteristic *pHeartRateControlPointCharacteristic;
BLECharacteristic *pBatteryCharacteristics;
BLECharacteristic *pHandleBarWidthCharacteristic;

bool deviceConnected = false;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/


#define DEVICE_INFORMATION_UUID "0000180A-0000-1000-8000-00805F9B34FB"
#define SYSTEM_ID_UUID "00002a23-0000-1000-8000-00805f9b34fb"
#define SYSTEM_ID_VALUE_HEX 0x0BBA2DFEFF1A9EA0
#define MODEL_NUMBER_STRING_UUID "00002a24-0000-1000-8000-00805f9b34fb"
#define MODEL_NUMBER_STRING_VALUE "H7"
#define SERIAL_NUMBER_STRING_UUID "00002a25-0000-1000-8000-00805f9b34fb"
#define SERIAL_NUMBER_STRING_VALUE 20182DBA0B
#define SERIAL_NUMBER_STRING_VALUE_HEX 0x3230313832444241304200
#define FIRMWARE_REVISON_UUID "00002a26-0000-1000-8000-00805f9b34fb"
#define FIRMWARE_REVISON_VALUE "1.4.0"
#define HARDWARE_REVISION_UUID "00002a27-0000-1000-8000-00805f9b34fb"
#define HARDWARE_REVISION_VALUE_HEX 0x33393034343032342E313000
#define SOFTWARE_REVISION_UUID "00002a28-0000-1000-8000-00805f9b34fb"
#define SOFTWARE_REVISION_VALUE_HEX 0x483720332E312E3000
#define MANUFACTURER_NAME_STRING_UUID "00002a29-0000-1000-8000-00805f9b34fb"
#define MANUFACTURER_NAME_STRING_VALUE_HEX 0x506F6C617220456C656374726F204F7900

#define HEART_RATE_SERVICE_UUID        "0000180D-0000-1000-8000-00805F9B34FB"
#define HEART_RATE_CHARACTERISTIC_UUID "00002a37-0000-1000-8000-00805f9b34fb"
#define CLIENT_CHARCTERISTIC_CONFIG_DESCRIPTOR_UUID "00002902-0000-1000-8000-00805f9b34fb"

#define BODY_SENDOR_LOCATION_UUID "00002a38-0000-1000-8000-00805f9b34fb"
#define BODY_SENDOR_LOCATION_VALUE "1"

#define BATTERY_SERVICE_UUID "0000180f-0000-1000-8000-00805f9b34fb"
#define BATTERY_CHARACTERISTIC_UUID "00002A19-0000-1000-8000-00805F9B34FB"

#define HEART_RATE_CONTROL_POINT_UUID "00002a39-0000-1000-8000-00805f9b34fb"
#define HEART_RATE_CONTROL_POINT_VALUE "1"

#define GENERIC_ACCESS_UUID        "00001800-0000-1000-8000-00805F9B34FB"
#define GENERIC_ACCESS_DEVICE_NAME_CHARACTERISTIC_UUID "00002a00-0000-1000-8000-00805f9b34fb"

#define HANDLEBAR_WIDTH_SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define HANDLEBAR_WIDTH_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"


class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }

};

class MyWriterCallbacks: public BLECharacteristicCallbacks {

    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();
      handleBarWidth = atoi(value.c_str());
      timeout = 15000 + (int)(handleBarWidth * 29.1 * 2);
      EEPROM.write(0, handleBarWidth);
      EEPROM.commit();
    }
};



void setup() {
  Serial.begin(115200);
  
  // initialize EEPROM with predefined size
  EEPROM.begin(EEPROM_SIZE);

  handleBarWidth = EEPROM.read(0);
  timeout = 15000 + (int)(handleBarWidth * 29.1 * 2);

  // PIN-Modes
  pinMode(triggerPin, OUTPUT);
  pinMode(echoPin, INPUT);
  digitalWrite(triggerPin, HIGH);

  
  // Create the BLE Device
  BLEDevice::init("Polar H7 2DBA0B2F");

  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Services
  BLEService *pDeviceInfoService = pServer->createService(DEVICE_INFORMATION_UUID);
  BLEService *pHeartRateService = pServer->createService(HEART_RATE_SERVICE_UUID);
  BLEService *pBatteryLevelService = pServer->createService(BATTERY_SERVICE_UUID);
  BLEService *pHandleBarWidthService = pServer->createService(HANDLEBAR_WIDTH_SERVICE_UUID);

  
  // Create a BLE Characteristic
  pCharacteristic = pHeartRateService->createCharacteristic(
                      HEART_RATE_CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      //BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY //|
                      //BLECharacteristic::PROPERTY_INDICATE
                    );
 pHeartRateControlPointCharacteristic = pHeartRateService->createCharacteristic( HEART_RATE_CONTROL_POINT_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE
                      //BLECharacteristic::PROPERTY_NOTIFY //|
                      //BLECharacteristic::PROPERTY_INDICATE
                    );

 pHandleBarWidthCharacteristic = pHandleBarWidthService->createCharacteristic(HANDLEBAR_WIDTH_CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE
                      //BLECharacteristic::PROPERTY_NOTIFY //|
                      //BLECharacteristic::PROPERTY_INDICATE
                    );

 pSensorLocationCharacteristic = pHeartRateService->createCharacteristic(BODY_SENDOR_LOCATION_UUID, BLECharacteristic::PROPERTY_READ);
 uint8_t locationValue = 1;
 pSensorLocationCharacteristic->setValue(&locationValue,1);


 pSystemIDCharcateristic = pDeviceInfoService->createCharacteristic(SYSTEM_ID_UUID, BLECharacteristic::PROPERTY_READ);
 uint8_t systemIDValueHex[8] = {0x0B, 0xBA, 0x2D, 0xFE, 0xFF, 0x1A, 0x9E, 0xA0};
 pSystemIDCharcateristic->setValue(systemIDValueHex, 8);
 
 pModelNumberStringCharacteristic = pDeviceInfoService->createCharacteristic(MODEL_NUMBER_STRING_UUID, BLECharacteristic::PROPERTY_READ); 
 pModelNumberStringCharacteristic->setValue(MODEL_NUMBER_STRING_VALUE);

 pSerialNumberStringCharacteristic = pDeviceInfoService->createCharacteristic(SERIAL_NUMBER_STRING_UUID, BLECharacteristic::PROPERTY_READ);
 uint8_t serialNumberStringValueHex[11] = {0x32, 0x30, 0x31, 0x38, 0x32, 0x44, 0x42, 0x41, 0x30, 0x42, 0x00};
 pSerialNumberStringCharacteristic->setValue(serialNumberStringValueHex, 11);

 pFirmwareRevisionCharacteristic = pDeviceInfoService->createCharacteristic(FIRMWARE_REVISON_UUID, BLECharacteristic::PROPERTY_READ);
 pFirmwareRevisionCharacteristic->setValue(FIRMWARE_REVISON_VALUE);

 pHardwareRevisionCharacteristic = pDeviceInfoService->createCharacteristic(HARDWARE_REVISION_UUID, BLECharacteristic::PROPERTY_READ);
 uint8_t hardwareRevisionValueHex[12] = {0x33, 0x39, 0x30, 0x34, 0x34, 0x30, 0x32, 0x34, 0x2E, 0x31, 0x30, 0x00};
 pHardwareRevisionCharacteristic->setValue(hardwareRevisionValueHex, 12);

 pSoftwareRevisionCharacteristic = pDeviceInfoService->createCharacteristic(SOFTWARE_REVISION_UUID, BLECharacteristic::PROPERTY_READ);
 uint8_t softwareRevisionValueHex[9] = {0x48, 0x37, 0x20, 0x33, 0x2E, 0x31, 0x2E, 0x30, 0x00};
 pSoftwareRevisionCharacteristic->setValue(softwareRevisionValueHex, 9);

 pManufacturerNameCharacteristic = pDeviceInfoService->createCharacteristic(MANUFACTURER_NAME_STRING_UUID, BLECharacteristic::PROPERTY_READ);
 uint8_t manufacturerNameValueHex[17] = {0x50, 0x6F, 0x6C, 0x61, 0x72, 0x20, 0x45, 0x6C, 0x65, 0x63, 0x74, 0x72, 0x6F, 0x20, 0x4F, 0x79, 0x00};
 pManufacturerNameCharacteristic->setValue(manufacturerNameValueHex, 17);
 
#define MANUFACTURER_NAME_STRING_UUID "00002a29-0000-1000-8000-00805f9b34fb"
#define MANUFACTURER_NAME_STRING_VALUE_HEX 0x506F6C617220456C656374726F204F7900
 //Battery Service
 pBatteryCharacteristics = pBatteryLevelService->createCharacteristic(BATTERY_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ);
 pBatteryCharacteristics->setValue("90");
 
  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor
BLEDescriptor* pDescriptor = new BLEDescriptor(CLIENT_CHARCTERISTIC_CONFIG_DESCRIPTOR_UUID);


  pCharacteristic->addDescriptor(pDescriptor);
  uint8_t descriptorbuffer;
    descriptorbuffer = 1;
  pDescriptor->setValue(&descriptorbuffer,1);

 pHandleBarWidthCharacteristic->setCallbacks(new MyWriterCallbacks());
 char handleBarString[8];
 itoa(handleBarWidth, handleBarString, 10);
 pHandleBarWidthCharacteristic->setValue(handleBarString);  

  // Start the service
  pHeartRateService->start();
  pDeviceInfoService->start();
  pBatteryLevelService->start();
  pHandleBarWidthService->start();
    
  // Start advertising
  BLEAdvertisementData* data;
  pServer->getAdvertising()->addServiceUUID(pHeartRateService->getUUID());
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");
}

void loop() {
  CurrentTime = millis();
  uint8_t minDistance = 255;
  int measurements=0;
  while ((CurrentTime - StartTime) < measureInterval)
  {
    CurrentTime = millis();
    get_distance_min(minDistance);
    measurements++;
  }
  //float distance = get_distance();
  //float avg_distance = get_distance_avg();
  //int min_distance = get_distance_min();

  //Serial.write("distance: ");
  //Serial.print(distance) ;
  //Serial.write(" , avg. distance: ");
  //Serial.print(avg_distance) ;
  Serial.write("min. distance: ");
  Serial.print(minDistance) ;  
  Serial.write(" cm,");
  Serial.print(measurements);
  Serial.write(" measurents  \n"); 
  
  if (deviceConnected) {
    Serial.printf("*** NOTIFY: %d ***\n", minDistance);
    uint8_t buffer[2];
    uint8_t isit8or16bit = 0;

   buffer[0] = isit8or16bit;
   buffer[1] = minDistance;
   //buffer[2] = value;
    
    pCharacteristic->setValue(buffer, 2);
    pCharacteristic->notify();
  }

  unsigned long ElapsedTime = CurrentTime - StartTime;
  StartTime = CurrentTime;
  Serial.write(" Time elapsed: ");
  Serial.print(ElapsedTime);
  Serial.write(" milliseconds\n");
  //delay(2000);
}

float get_distance() {
  float duration=0;
  float distance=0;

  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  noInterrupts();
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);
  duration = pulseIn(echoPin, HIGH, timeout); // Erfassung - Dauer in Mikrosekunden
  interrupts();

  distance = (duration / 2) / 29.1; // Distanz in CM
  return(distance);
}

float get_distance_avg() {
  float alt = 0;
  float avg;
  float dist;
  int i;

  delay(10);
  alt = get_distance();
  delay(10);
  for (i=0; i<runs; i++) {
    dist = get_distance();
    avg = (0.8*alt) + (0.2*dist);
    alt = avg;
    delay(10);
  }
  return (avg);
}

uint8_t get_distance_min() {
  
  float min=255.0;
  float dist;
  int i;

  delay(10);
  for (i=0; i<runs; i++) {
    dist = get_distance()-42.4;
    if ((dist > 0.0) && (dist < min))
    {
      min=dist;
    }
    delay(20);
  }
  return (uint8_t(min));
}

void get_distance_min(uint8_t& min_distance) {
  float dist;
  dist = get_distance()-float(handleBarWidth);
    if ((dist > 0.0) && (dist < float(min_distance)))
    {
      min_distance=uint8_t(dist);
    }
    delay(20);
}

