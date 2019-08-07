# ESP32 BLE Distance Measurements

Distance Measurement HC-SR04 sensors connected to ESP32 pretending to be heart rate sensor for logging with fitness apps.

## Description

Inspired by the Berlin project Radmesser, this version connects to tracking apps by pretending to be a heart rate sensor. This enables you to write the distance to a gpx file and use it later in video editing software like [Dashware](http://www.dashware.net/) or [Garmin Virb Edit](https://buy.garmin.com/de-DE/DE/p/573412) to show the distance in your video.

## Getting Started

### Hardware

The prototype by [Zweirat](https://zweirat-stuttgart.de/projekte/radmesser/) used the following:
* [ESP32](https://www.az-delivery.de/products/esp32-developmentboard)
* [LiFePo-Battery](https://www.akkuteile.de/lifepo-akkus/18650/a123-apr18650m-a1-1100mah-3-2v-3-3v-lifepo4-akku/a-1006861/)
* [Battery-Protection-Board](https://www.ebay.de/itm/202033076322)
* [LiFePo charging module](https://www.ebay.de/itm/MicroUSB-TP5000-3-6v-1A-Charger-Module-3-2v-LiFePO4-Lithium-Battery-Charging-/122164745507)
* [HC-SR04P](https://www.ebay.de/itm/183610614563)

With LiFePo Batteries you can power the device directly and avoid step-up/step-down losses that you have when using a powerbank. On the other hand you need the HC-SR04P instead of the HC-SR04 sensor if you don't have 5V power supply.

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
* power up device
* Use [BLE Scanner](https://play.google.com/store/apps/details?id=com.macdom.ble.blescanner) or your favorite BLE app to send ond permanently save offset (for example the distance from sensor to handlebar)
* Start your favourite tracking app
Good results were made with [RunnerUp](https://play.google.com/store/apps/details?id=org.runnerup&hl=de), Strava had some issues on Android, look here for [alternatives](https://play.google.com/store/apps/details?id=org.runnerup&hl=de), no experience with iOS so far. 
* Connect heart rate sensor

### ToDo
* Write nice tutorial how to process video data and create synchronized overlays
* Develop an own app to record videos in loop mode and only save short clips when distance is low. At the end of the ride, ask user to confirm that a car was overtaking. Possibility to create overlays for video and report anonymous metadata of overtaking incidents (good and bad ones) to open data platform.

## Acknowledgments

Inspiration, code snippets, etc.
* [Neil Kolbans ESP32 BLE Library](https://github.com/nkolban/ESP32_BLE_Arduino)
* [Wiring up the Sensor](https://www.smarthomeng.de/entfernungsmessung-auf-basis-eines-esp32-und-smarthomeng)
