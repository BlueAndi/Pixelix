AnimatedGIFs replacement
------------------------
Please go to 
- http://marc.merlins.org/perso/arduino/post_2018-07-13_AnimatedGIFs-for-SmartMatrix-or-NeoMatrix-_Neopixel-WS2812B_-from-SDcard-or-SPIFFS_-on-Teensy-3_x_-ESP8266_-or-ESP32.html
- https://github.com/marcmerlin/AnimatedGIFs

for a better/newer version of this code.

You may however like this version if you really want a bare bones gif decoder.


Original Text
-------------
Demo to display gifs read from SPIFFS from ESP8266 (should also work on ESP32).
 
Decoding engine from Jason Coon, Aurora.
https://github.com/pixelmatix/aurora/

The gifs need to be uploaded to the ESP8266's SPIFFS.  You can do this within the Arduino IDE after installing the [Arduino ESP8266FS tool](http://esp8266.github.io/Arduino/versions/2.3.0/doc/filesystem.html#uploading-files-to-file-system).

With ESP8266FS installed upload the web app using `ESP8266 Sketch Data Upload` command in the Arduino Tools menu.


