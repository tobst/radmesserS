class Sensor
{
  public:
    Sensor() {}
    virtual ~Sensor() {}
    virtual void init() = 0;

  protected:

  private:
};

class DistanceSensor : public Sensor
{
  public:
    DistanceSensor() : Sensor() {
    }
    ~DistanceSensor() {}
    virtual void init()
    {

    }
    void getMinDistance(uint8_t& min_distance);
    virtual float getDistance() = 0;
    void setOffset(uint8_t offset) {
      m_offset = offset;
    }
  protected:
    uint8_t m_offset = 0;
  private:
    unsigned long m_measureInterval = 1000;

};

class HCSR04DistanceSensor : public DistanceSensor
{
  public:
    HCSR04DistanceSensor() : DistanceSensor() {
      pinMode(m_triggerPin, OUTPUT);
      pinMode(m_echoPin, INPUT);
      digitalWrite(m_triggerPin, HIGH);
    }
    ~HCSR04DistanceSensor() {}
    void init() {
    }
    float getDistance();
    void setOffset(uint8_t offset) {
      m_offset = offset;
      m_timeout = 15000 + (int)(m_offset * 29.1 * 2);
    }
  protected:
  private:
    int m_triggerPin = 15;
    int m_echoPin = 4;
    int m_timeout = 15000;
};
