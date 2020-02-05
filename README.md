
# ESPHomeController
Generic IOT platform for Home Automation based on the ESP8266/ESP32.



After a few times of implementing IOT devices for home automation, i have recognized that each new one looks better and better, but this is always copy-paste approach... And not so easilly to back to previous implementation and merge innovation. For instance after implementing simple IOT for Relay switching I have did another project to control RGB strip (WS2811/2812 ) and finally get to sketches which I have to support... 
Instead of that i decided to develop universal sketch to support code once, but with possibility to upload to different IOT controllers for my Home Automation.
**Main targets are :**
 1. Support ESP32 and ESP8266  with the same code
 2. The same skecth should support different devices, the difference must be outside in the configuration file. Like Lego bricks...
 3. Updatable via the web (OTA), some times not easilly to physically access device, when it's altready installed
 4. Nice web interface to controll devices as well to setup them
 5. RAW file browser to upload/change configuration files or HTML for the WEB
 6. MQTT integration to be able control devices via Home Kit
 7. Native support of Apple Home Kit integration (only for ESP32)
 8. On board automation scripts/triggers.. Some times is not easy to setup this one via Home Kit or Home Kit doesn't exists
 9. Configuration portal after firts start (captive portal)
 10. Suports of RF 433 Mhz control by any existing transmitters
 11. More and more
 
Ok, let's describe basic thing how it works
Major element of this solution is **service**, like in windows unix , android etc. Each service are responsible to control one device wired to ESP. Hovewer it's possible to run 2 instances of the same service to control similar devices. List if services is configurable by json file with their properties
Second and maybe last major element is **trigger** . It define interaction between services and fulfill automation. The list of of triggers as well configurabel

**Few examples:**

 1. You need device to control only one *Relay* (for any reason: light, heater,...) Just define configuration services.json with  one service RelayController, define pin, where relay is physically wired, that's all ! You will get a device ready and able to controll your lights via Web or Home Kit (Home Kit integration possibilities will be explained later)
2.  Additionally to previos you want to control your *Relay* by *Light sensor* . Yo need to add LDRController to listen wired LDR. Web and Home will automatically detect this and you can see your lumens over the or Home Kit). But you can configure LDR to Realy trigger with barier values, and than when LDR is higher that barier trigger will send command to Relay to switch off or vice verse..
3. More and more examples and combination within services and triggers...will come


### Used External Libraries

This project uses libraries and code by different authors:

- [WiFiManager](https://github.com/tzapu/WiFiManager) by tzapu (tested with version 0.12.0)

- [WS2812FX](https://github.com/kitesurfer1404/WS2812FX) by kitesurfer1404 (tested with version downloaded 2017-02-05)



- [Adafruit NeoPixel](https://github.com/adafruit/Adafruit_NeoPixel) by adafruit (tested with 1.1.2)

- [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer)

 -[rc-switch](https://github.com/sui77/rc-switch) small changes are done to works with ESP32 as well
 
 -[ESPHap](https://github.com/Yurik72/ESPHap) this is library for native Apple Home Kit integration

Parts of the code were taken or inspired by the following sources:

 - [SPIFFS Webserver](https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WebServer/examples/FSBrowser) by Hristo Gochkov
 This part is adapted fo the following following staff:
	 
	 - Supports ESP32 file system
	 - Enhanced filebrowse.html
	 - "Prefly" request and headers for cross domain areas
	 - overwritten within new code when skecth is uses AsyncWebServer 

Thank you to all the authors for distributing their software that way.
I hope I didn't miss any sources and mentioned every author. In case I forgot someone please let me know and I will fix it.

**Goals are reached :**

 - Adapted *ESP8266HTTPUpdateServer* , now it's supports ESP32. Have a look of *ESP32HTTPUpdateServer.h/cpp* implementatation with traditional WebServer;
 - Adapted *ESP8266HTTPUpdateServer* for working withis [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer). Have a look of  *ESPAsyncUpdateServer.h/cpp* 
 - Second core of ESP32 can be used with proper service configuration. Now a few services can run on second core of ESP32;
 - Built-in web site implemented on React js, and browser content is very fast, compatible with native mobile application, Thanks to React developers;
- Sketch can be compiled within VS 2017, by using  [Arduino IDE for Visual Studio](https://marketplace.visualstudio.com/items?itemName=VisualMicro.ArduinoIDEforVisualStudio)
- Adapt WS2812FX library to perfectly works on ESP32, see details [here](https://github.com/Yurik72/ESPHomeController/wiki/WS2812-driver-to-remove-flickering)

**WebSite**

 ESPHomeController has own web site. Ready sources located in the data directory. You just need to upload them into your spiff.
 This Web site is builded on ReactJS, therefore to change anything you need to use [sources](https://github.com/Yurik72/ESPHomeController/tree/master/WebSource)


For technical infomation see [Wiki](https://github.com/Yurik72/ESPHomeController/wiki)

Please have a look https://github.com/Yurik72/esphapcontroller  , there is a clone of code base adapted for esp-idf and NATIVE support of Apple Home, this is version based on ESP-IDF

**Projects using this firmware**

[fireplace lamp with tem sensors](https://www.instructables.com/id/Accu-Multicololred-LED-Lamp-With-Weather/)
[Bed Room Lamp Ws2812](https://www.instructables.com/id/Bed-Room-Lamp-Ws2812/)
[Weather station](https://www.instructables.com/id/ESP32-Weather-Station-Solar-Powered/)


P.S. !! This is starting project not all parts already comitted. Will come soon after testing. If you are interested feel free to contact me
