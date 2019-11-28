#include <TM1637Display.h>
#include "SSD1306.h"
#include "font.h"

const int CLK = 33; //Set the CLK pin connection to the display
const int DIO = 25; //Set the DIO pin connection to the display
//Segments for line of dashes on display
uint8_t segments[] = {64, 64, 64, 64};

//TM1637Display display(CLK, DIO);

//SSD1306  displayOLED(0x3c, 21, 22);

/*


*/

class DisplayDevice
{
  public:
    DisplayDevice() {}
    virtual ~DisplayDevice() {}
    virtual void showValue(uint8_t) = 0;

  protected:
    virtual void init() = 0;
    virtual void clear() = 0;
    virtual void setMaxBrightness() = 0;



};

class TM1637DisplayDevice : public DisplayDevice
{
  public:
    TM1637DisplayDevice() : DisplayDevice() {
      m_display = new TM1637Display(CLK, DIO);
      m_display->setBrightness(0x07);
    }
    ~TM1637DisplayDevice() {
      delete m_display;
    }
    void showValue(uint8_t value)
    {
      if (value == 255)
      {
        m_display->setSegments(segments);
      }
      else
      {
        m_display->showNumberDec(value);
      }
    }
  protected:
    void init()
    {
      m_display->setBrightness(0x07);
    }
    void clear() {

    }
    void setMaxBrightness() {

    }

  private:
    TM1637Display* m_display;

};

class SSD1306DisplayDevice : public DisplayDevice
{
  public:
    //SSD1306  displayOLED(0x3c, 21, 22);
    SSD1306DisplayDevice() : DisplayDevice() {
      m_display = new SSD1306(0x3c, 21, 22);
      m_display->init();
      m_display->setBrightness(255);
      m_display->setFont(Dialog_plain_50);
    }
    ~SSD1306DisplayDevice() {
      delete m_display;
    }
    void showValue(uint8_t value)
    {
      m_display->clear();
      if (value == 255)
      {
        m_display->drawString(0, 0, "---");
      }
      else
      {
        m_display->drawString(0, 0, String(value));
      }
      m_display->display();
    }
  protected:
    void init()
    {
      m_display->init();
      m_display->setBrightness(255);
      m_display->setFont(Dialog_plain_50);
    }
    void clear() {

    }
    void setMaxBrightness() {

    }
  private:
    SSD1306* m_display;
    uint8_t m_value;
};

