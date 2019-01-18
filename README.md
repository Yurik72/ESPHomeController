
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
 7. On board automation scripts/triggers.. Some times is not easy to setup this one via Home Kit or Home Kit doesn't exists
 8. Configuration portal after firts start (captive portal)
 9. More and more
 
Ok, let's describe basic thing how it works
Major element of this solution is **service**, like in windows unix , android etc. Each service are responsible to control one device wired to ESP. Hovewer it's possible to run 2 instances of the same service to control similar devices. List if services is configurable by json file with their properties
Second and maybe last major element is **trigger** . It define interaction between services and fulfill automation. The list of of triggers as well configurabel

**Few examples:**

 1. You need device to control only one *Relay* (for any reason: light, heater,...) Just define configuration services.json with  one service RelayController, define pin, where relay is physically wired, that's all ! You will get a device ready and able to controll your lights via Web or Home Kit (Home Kit integration possibilities will be explained later)
2.  Additionally to previos you want to control your *Relay* by *Light sensor* . Yo need to add LDRController to listen wired LDR. Web and Home will automatically detect this and you can see your lumens over the or Home Kit). But you can configure LDR to Realy trigger with barier values, and than when LDR is higher that barier trigger will send command to Relay to switch off or vice verse..
3. More and more examples and combination within services and triggers...will come

