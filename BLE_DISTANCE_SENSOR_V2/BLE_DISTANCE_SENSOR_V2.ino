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
#define CIRCULAR_BUFFER_INT_SAFE
#include <CircularBuffer.h>
#include "displays.h"
#include "FS.h"
#include "SD.h"
#include "SPI.h"

//#include <GPX.h>

//GPX myGPX;

#include "gps.h"
#include "ble.h"
#include "vector.h"
#include "writer.h"
#include "sensor.h"

//GPS

// define the number of bytes to store
#define EEPROM_SIZE 1
#define EEPROM_SIZE 128

#define MAX_SENSOR_VALUE 255

// PINs
const int triggerPin = 15;
const int echoPin = 4;

const int PushButton = 2;


// VARs
const int runs = 20;
unsigned long measureInterval = 1000;
unsigned long timeOfLastNotificationAttempt = millis();
unsigned long timeOfMinimum = millis();
unsigned long StartTime = millis();
unsigned long CurrentTime = millis();
unsigned long buttonPushedTime = millis();

int timeout = 15000;
bool usingSD = false;
String text = "";
uint8_t minDistanceToConfirm = MAX_SENSOR_VALUE;
uint8_t confirmedMinDistance = MAX_SENSOR_VALUE;
bool transmitConfirmedData = false;
int lastButtonState = 0;
int buttonState = 0;
bool handleBarWidthReset = false;

String filename;

CircularBuffer<DataSet*, 10> dataBuffer;
Vector<String> sensorNames;

#define TASK_SERIAL_RATE 1000 // ms

uint32_t nextSerialTaskTs = 0;
uint32_t nextOledTaskTs = 0;

uint8_t handleBarWidth = 0;
bool deviceConnected = false;

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

DisplayDevice* displayTest;
//DisplayDevice* displayTest2;

FileWriter* writer;

DistanceSensor* sensor1;
// See the following for generating UUIDs:
// https://www.uuidgenerator.net/


void setup() {
  String sensorName1 = "DistanceLeft";
  sensorNames.push_back(sensorName1);
  displayTest = new TM1637DisplayDevice;
  //displayTest2 = new SSD1306DisplayDevice;

  sensor1 = new HCSR04DistanceSensor;

  //GPS
  SerialGPS.begin(9600, SERIAL_8N1, 16, 17);
  while (!EEPROM.begin(EEPROM_SIZE)) {
    true;
  }

  readLastFixFromEEPROM();

  Serial.begin(115200);
  while (!SD.begin())
  {
    Serial.println("Card Mount Failed");
    delay(1000);
  }

  {
    Serial.println("Card Mount Succeeded");
    writer = new CSVFileWriter;
    writer->setFileName();
    writer->writeHeader();
  }
  Serial.println("File initialised");

  // initialize EEPROM with predefined size
  EEPROM.begin(EEPROM_SIZE);

  sensor1->setOffset(EEPROM.read(0));

  // PIN-Modes
  pinMode(PushButton, INPUT);

  while (gps.satellites.value() < 4)
  {
  readGPSData();
  delay(1000);
  Serial.println("Waiting for GPS fix... \n");
  }
  Serial.println("Got GPS Fix  \n");
  heartRateBLEInit();
  Serial.println("Waiting a client connection to notify...");
}

/*
   easily read and write EEPROM
*/
template <class T> int EEPROM_writeAnything(int ee, const T& value)
{
  const byte* p = (const byte*)(const void*)&value;
  int i;
  for (i = 0; i < sizeof(value); i++)
    EEPROM.write(ee++, *p++);
  return i;
}

template <class T> int EEPROM_readAnything(int ee, T& value)
{
  byte* p = (byte*)(void*)&value;
  int i;
  for (i = 0; i < sizeof(value); i++)
    *p++ = EEPROM.read(ee++);
  return i;
}

void loop() {

  DataSet* currentSet = new DataSet;
  writeLastFixToEEPROM();
  // Todo: proceed only if gps is valid and updated
  readGPSData();
  currentSet->location = gps.location;
  currentSet->altitude = gps.altitude;
  currentSet->date = gps.date;
  currentSet->time = gps.time;

  CurrentTime = millis();
  uint8_t minDistance = MAX_SENSOR_VALUE;
  int measurements = 0;
  if ((CurrentTime - timeOfMinimum ) > 5000)
  {
    minDistanceToConfirm = MAX_SENSOR_VALUE;
  }

  while ((CurrentTime - StartTime) < measureInterval)
  {
    CurrentTime = millis();
    sensor1->getMinDistance(minDistance);

    if (minDistance < minDistanceToConfirm)
    {
      minDistanceToConfirm = minDistance;
      timeOfMinimum = millis();
    }
    //displayTest->showValue(minDistanceToConfirm);
    displayTest->showValue(gps.satellites.value());
    //displayTest2->showValue(minDistanceToConfirm);

    if ((minDistanceToConfirm < MAX_SENSOR_VALUE) && !transmitConfirmedData)
    {
      buttonState = digitalRead(PushButton);
      if (buttonState != lastButtonState)
      {
        if (buttonState == LOW)
        {
          if (!handleBarWidthReset) transmitConfirmedData = true;
          else handleBarWidthReset = false;
        }
        else
        {
          buttonPushedTime = CurrentTime;
        }
      }
      else
      {
        if (buttonState == HIGH)
        {
          //Button state is high for longer than 5 seconds -> reset handlebar width
          //Todo: do it for all connected sensors 
          if ((CurrentTime - buttonPushedTime > 5000))
          {
            setHandleBarWidth(minDistanceToConfirm);
            displayTest->showValue(minDistanceToConfirm);
            //displayTest2->showValue(minDistanceToConfirm);
            delay(5000);
            handleBarWidthReset = true;
            transmitConfirmedData = false;
          }
        }
      }
      lastButtonState = buttonState;
    }
    measurements++;
  }
  currentSet->sensorValues.push_back(minDistance);

  //if nothing was detected, write the dataset to file, otherwise write it to the buffer for confirmation
  if ((minDistance == MAX_SENSOR_VALUE) && dataBuffer.isEmpty())
  {
    if (writer) writer->writeData(currentSet);
    delete currentSet;
  }
  else
  {
    dataBuffer.unshift(currentSet);
  }

  Serial.write("min. distance: ");
  Serial.print(minDistance) ;
  Serial.write(" cm,");
  Serial.print(measurements);
  Serial.write(" measurements  \n");

  uint8_t buffer[2];
  uint8_t isit8or16bit = 0;
  buffer[0] = isit8or16bit;
  if (transmitConfirmedData)
  {
    buffer[1] = minDistanceToConfirm;
    // make sure the minimum distance is saved only once
    using index_t = decltype(dataBuffer)::index_t;
    index_t j;
    for (index_t i = 0; i < dataBuffer.size(); i++)
    {
      if (dataBuffer[i]->sensorValues[0] == minDistanceToConfirm)
      {
        j = i;
      }
    }
    for (index_t i = 0; i < dataBuffer.size(); i++)
    {
      if (i != j)
      {
        dataBuffer[i]->sensorValues[0] = MAX_SENSOR_VALUE;
      }
    }

    while (!dataBuffer.isEmpty())
    {
      DataSet* dataset = dataBuffer.shift();
      Serial.printf("Trying to write set to file\n");
      if (writer) writer->writeData(dataset);
      Serial.printf("Wrote set to file\n");
      delete dataset;
    }
    minDistanceToConfirm = MAX_SENSOR_VALUE;
    transmitConfirmedData = false;
  }
  else
  {
    buffer[1] = MAX_SENSOR_VALUE;
  }

  if (dataBuffer.isFull())
  {
    DataSet* dataset = dataBuffer.shift();
    dataset->sensorValues[0] = MAX_SENSOR_VALUE;
    Serial.printf("Buffer full, writing set to file\n");
    if (writer) writer->writeData(dataset);
    Serial.printf("Deleting dataset\n");
    delete dataset;
  }

  if (deviceConnected) {
    Serial.printf("*** NOTIFY: %d ***\n", buffer[1] );
    heartRateBLENotify(buffer);
  }

  unsigned long ElapsedTime = CurrentTime - StartTime;
  StartTime = CurrentTime;
  Serial.write(" Time elapsed: ");
  Serial.print(ElapsedTime);
  Serial.write(" milliseconds\n");
}
