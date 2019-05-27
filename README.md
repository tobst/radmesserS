# ESP32 BLE Distance Measurements

Distance Measurement HC-SR04 sensors connected to ESP32 pretending to be heart rate sensor for logging with fitness apps.

## Description

Inspired by the Berlin project Radmesser, this version connects to tracking apps by pretending to be a heart rate sensor. This enables you to write the distance to a gpx file and use it later in video editing software like [dashware](http://www.dashware.net/) to show the distance in your video.

## Getting Started

### Hardware

The prototype by Zweirat used the following:
* [ESP32](https://www.az-delivery.de/products/esp32-developmentboard)
* [LiFePo-Battery](https://www.akkuteile.de/lifepo-akkus/18650/a123-apr18650m-a1-1100mah-3-2v-3-3v-lifepo4-akku/a-1006861/)
* [Battery-Protection-Board](https://www.ebay.de/itm/202033076322)
* [LiFePo charging module](https://www.ebay.de/itm/MicroUSB-TP5000-3-6v-1A-Charger-Module-3-2v-LiFePO4-Lithium-Battery-Charging-/122164745507)
* [HC-SR04P](https://www.ebay.de/itm/183610614563)

### Dependencies

* ESP32 device driver
* Neil Kolbans ESP32 BLE Library
* Arduino IDE

### Installing

* clone or download repository
* open in Arduino IDE
* compile and upload to ESP32
* Connect sensor

### Executing program

* Start your favourite tracking app
* Connect heart rate sensor


## Acknowledgments

Inspiration, code snippets, etc.
* [Neil Kolbans ESP32 BLE Library](https://github.com/nkolban/ESP32_BLE_Arduino)
* [Wiring up the Sensor](https://www.smarthomeng.de/entfernungsmessung-auf-basis-eines-esp32-und-smarthomeng)
