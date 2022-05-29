# TalkingClock
Based on the Tetris Clock but with mp3 player

Updated version 1.1:
Access point **bug** fixed: Name of access point now correct: Geniusweb
wifi settings **bug** fixed: name and password can now be up to 35 characters
Wifi **bug** fixed: with wpa protocol, system won't boot to wifi without disconnecting. Was related to the order in which things are initialized.


If your display is not working, maybe you have installed the wrong version of the I2s Display driver.
You can test it by using the webprogrammer ( with Chrome or edge only) if the display is working when you are using this pre compiled version it means you have a library version incompatibility
https://donnersm.github.io/Webflash-FFT/flash.html

Note: If you are using the sketch version to compile it yourself, you'll need to install some libaries. Most are available in the arduino library manager. The audio Lib can be found here:
https://github.com/schreibfaul1/ESP32-audioI2S
I used version 2.00

Other libs that i used, here are the version numbers. Install one at a time, some libs are included in the ESP32 framework and don't need to be installed.
Using library FastLED_NeoMatrix at version 1.1 
Using library Framebuffer_GFX at version 1.0 i
Using library FastLED at version 3.4.0 
Using library Adafruit_GFX_Library at version 1.10.4  
Using library WiFi at version 1.0 
Using library WebServer at version 1.0 
Using library WebSockets at version 2.1.4 
Using library WiFiClientSecure at version 1.0 
Using library ArduinoJson at version 6.19.1 
Using library FS at version 1.0 
Using library SPIFFS at version 1.0 
Using library ESP32_HUB75_LED_MATRIX_PANEL_DMA_Display at version 2.0.5 
Using library TetrisAnimation at version 1.1.0 
Using library ezTime at version 0.8.3 
Using library ESP32-audioI2S-master at version 2.0.0 
Using library SPI at version 1.0 
Using library SD at version 1.0.5 
Using library SD_MMC at version 1.0 
Using library FFat at version 1.0 
Using library Adafruit_BusIO at version 1.7.1 
Using library Wire at version 1.0.1 
Using library EEPROM at version 1.0.3 


I got some feedback that the main branch audio driver plays back the audio with a stutter. If this is the case, then you can use this branch instead:
https://github.com/schreibfaul1/ESP32-audioI2S/tree/prepare-for-m4a
