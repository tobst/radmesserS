void readGPSData() {
   gpsState.distMax = 0;
  gpsState.altMax = -999999;
  gpsState.spdMax = 0;
  gpsState.altMin = 999999;

  while (SerialGPS.available() > 0) {
    gps.encode(SerialGPS.read());
  }
  if (gps.satellites.value() > 4) {
    gpsState.dist = TinyGPSPlus::distanceBetween(gps.location.lat(), gps.location.lng(), gpsState.originLat, gpsState.originLon);

    if (gpsState.dist > gpsState.distMax && abs(gpsState.prevDist - gpsState.dist) < 50) {
      gpsState.distMax = gpsState.dist;
    }
    gpsState.prevDist = gpsState.dist;

    if (gps.altitude.meters() > gpsState.altMax) {
      gpsState.altMax = gps.altitude.meters();
    }

    if (gps.speed.kmph() > gpsState.spdMax) {
      gpsState.spdMax = gps.speed.kmph();
    }

    if (gps.altitude.meters() < gpsState.altMin) {
      gpsState.altMin = gps.altitude.meters();
    }
  }
  if (nextSerialTaskTs < millis()) {
    Serial.print("LAT=");  Serial.println(gps.location.lat(), 6);
    Serial.print("LONG="); Serial.println(gps.location.lng(), 6);
    Serial.print("ALT=");  Serial.println(gps.altitude.meters());
    Serial.print("Sats=");  Serial.println(gps.satellites.value());
    Serial.print("DST: ");
    Serial.println(gpsState.dist, 1);
    nextSerialTaskTs = millis() + TASK_SERIAL_RATE;
  }

}

