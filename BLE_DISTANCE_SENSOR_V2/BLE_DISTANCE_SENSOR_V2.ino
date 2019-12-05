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
#include "displays.h"
#include "FS.h"
#include "SD.h"
#include "SPI.h"

//#include <GPX.h>

//GPX myGPX;

#include "gps.h"
#include "ble.h"
#include "writer.h"
//GPS


// define the number of bytes to store
#define EEPROM_SIZE 1
#define EEPROM_SIZE 128

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

int timeout = 15000;
bool usingSD = false;
String text = "";
uint8_t minDistanceToConfirm = 255;
uint8_t confirmedMinDistance = 255;
bool transmitConfirmedData = false;

String filename;


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
DisplayDevice* displayTest2;

FileWriter* writer;
// See the following for generating UUIDs:
// https://www.uuidgenerator.net/


void setup() {

  displayTest = new TM1637DisplayDevice;
  displayTest2 = new SSD1306DisplayDevice;
  writer = new CSVFileWriter;

  //GPS
  SerialGPS.begin(9600, SERIAL_8N1, 16, 17);
  while (!EEPROM.begin(EEPROM_SIZE)) {
    true;
  }

  /*
     Die drei Achsrichtungen x, y, z aus dem Speicher
     lesen und hinterlegen
  */
  long readValue;
  EEPROM_readAnything(4, readValue);
  gpsState.originLat = (double)readValue / 1000000;

  EEPROM_readAnything(8, readValue);
  gpsState.originLon = (double)readValue / 1000000;

  EEPROM_readAnything(12, readValue);
  gpsState.originAlt = (double)readValue / 1000000;



  Serial.begin(115200);
  if (!SD.begin())
  {
    Serial.println("Card Mount Failed");
  }
  else
  {
    usingSD = true;
    int fileSuffix = 0;
    String base_filename = "/sensorData";
    filename = base_filename + String(fileSuffix) + ".txt";
    while (SD.exists(filename.c_str()))
    {
      fileSuffix++;
      filename = base_filename + String(fileSuffix) + ".txt";
    }
    writer->writeFile(SD, filename.c_str(), "Data \n ");
  }

  // initialize EEPROM with predefined size
  EEPROM.begin(EEPROM_SIZE);

  handleBarWidth = EEPROM.read(0);
  timeout = 15000 + (int)(handleBarWidth * 29.1 * 2);

  // PIN-Modes
  pinMode(triggerPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(PushButton, INPUT);
  digitalWrite(triggerPin, HIGH);

  heartRateBLEInit();

  Serial.println("Waiting a client connection to notify...");
}

/*
   Hilfsfunktionen um vereinfacht Speicher zu lesen und schreiben
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

  static int p0 = 0;

  // GPS Koordinaten von Modul lesen
  gpsState.originLat = gps.location.lat();
  gpsState.originLon = gps.location.lng();
  gpsState.originAlt = gps.altitude.meters();

  // Aktuelle Position in nichtflüchtigen ESP32-Speicher schreiben
  long writeValue;
  writeValue = gpsState.originLat * 1000000;
  EEPROM_writeAnything(4, writeValue);
  writeValue = gpsState.originLon * 1000000;
  EEPROM_writeAnything(8, writeValue);
  writeValue = gpsState.originAlt * 1000000;
  EEPROM_writeAnything(12, writeValue);
  EEPROM.commit(); // erst mit commit() werden die Daten geschrieben

 readGPSData();
 
  if (usingSD)
  {
    text += "\n";
    text += String(millis());
    text += ";";
    text += String(gps.location.lat(), 6);
    text += ";";
    text += String(gps.location.lng(), 6);
    text += ";";
    writer->appendFile(SD, filename.c_str(), text.c_str() );
    text = "";
  }

  CurrentTime = millis();
  uint8_t minDistance = 255;
  int measurements = 0;
  if ((CurrentTime - timeOfMinimum ) > 5000)
  {
    minDistanceToConfirm = 255;
  }

  while ((CurrentTime - StartTime) < measureInterval)
  {
    CurrentTime = millis();
    get_distance_min(minDistance);

    if (minDistance < minDistanceToConfirm)
    {
      minDistanceToConfirm = minDistance;
      timeOfMinimum = millis();
    }
    displayTest->showValue(minDistanceToConfirm);
    displayTest2->showValue(minDistanceToConfirm);

    if ((minDistanceToConfirm < 255) && !transmitConfirmedData)
    {
      transmitConfirmedData = digitalRead(PushButton);
    }
    measurements++;
  }
  text += String(minDistance);
  text += ";";
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
  Serial.write(" measurements  \n");

  uint8_t buffer[2];
  uint8_t isit8or16bit = 0;
  buffer[0] = isit8or16bit;
  if (transmitConfirmedData)
  {
    buffer[1] = minDistanceToConfirm;
    String confirmed = "\nDistance confirmed:" + String(minDistanceToConfirm) + "\n" + String(millis()) + ";";
    minDistanceToConfirm = 255;
    transmitConfirmedData = false;
    writer->appendFile(SD, filename.c_str(), confirmed.c_str() );
  }
  else
  {
    buffer[1] = 255;
  }

  if (deviceConnected) {

    Serial.printf("*** NOTIFY: %d ***\n", buffer[1] );
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
  float duration = 0;
  float distance = 0;

  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  noInterrupts();
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);
  duration = pulseIn(echoPin, HIGH, timeout); // Erfassung - Dauer in Mikrosekunden
  interrupts();

  distance = (duration / 2) / 29.1; // Distanz in CM
  return (distance);
}

float get_distance_avg() {
  float alt = 0;
  float avg;
  float dist;
  int i;

  delay(10);
  alt = get_distance();
  delay(10);
  for (i = 0; i < runs; i++) {
    dist = get_distance();
    avg = (0.8 * alt) + (0.2 * dist);
    alt = avg;
    delay(10);
  }
  return (avg);
}

uint8_t get_distance_min() {

  float min = 255.0;
  float dist;
  int i;

  delay(10);
  for (i = 0; i < runs; i++) {
    dist = get_distance() - 42.4;
    if ((dist > 0.0) && (dist < min))
    {
      min = dist;
    }
    delay(20);
  }
  return (uint8_t(min));
}

void get_distance_min(uint8_t& min_distance) {
  float dist;
  dist = get_distance() - float(handleBarWidth);
  if ((dist > 0.0) && (dist < float(min_distance)))
  {
    min_distance = uint8_t(dist);
  }
  else
  {
    dist = 0.0;
  }
  if (usingSD)
  {
    //text += String(dist);
    //text += ";";
  }
  delay(20);
}

