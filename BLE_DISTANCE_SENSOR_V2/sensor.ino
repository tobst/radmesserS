void DistanceSensor::getMinDistance(uint8_t& min_distance) {
  float dist;
  dist = getDistance() - float(m_offset);
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

float HCSR04DistanceSensor::getDistance() {
  float duration = 0;
  float distance = 0;

  digitalWrite(m_triggerPin, LOW);
  delayMicroseconds(2);
  noInterrupts();
  digitalWrite(m_triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(m_triggerPin, LOW);
  duration = pulseIn(m_echoPin, HIGH, m_timeout); // Erfassung - Dauer in Mikrosekunden
  interrupts();

  distance = (duration / 2) / 29.1; // Distanz in CM
  return (distance);
}
