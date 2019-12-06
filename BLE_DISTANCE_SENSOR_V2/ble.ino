
void heartRateBLEInit() {
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
  pSensorLocationCharacteristic->setValue(&locationValue, 1);


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
  pDescriptor->setValue(&descriptorbuffer, 1);

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
}


void heartRateBLENotify(uint8_t buffer[2]) {
  pCharacteristic->setValue(buffer, 2);
  pCharacteristic->notify();
}

