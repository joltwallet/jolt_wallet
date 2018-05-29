How To Setup Generic Dev Board
======================================

Introduction
---------------------
This is a howto guide on how to setup the a Generic (e.g. MakerFocus/MakerHawk) ESP32 Development Board with built in wifi to create your own Jolt Wallet. It will require some soldering, a small amount of electronics construction and then building a development environment to then flash the wallet. 

Order Components
---------------------
* 1x MakerFocus ESP32 Development Board WiFi with 0.96inch OLED Display WIFI Kit32 Arduino Compatible CP2012 for Arduino Nodemcu - http://a.co/hHROZ2E
* 4x 6mmx6mm Momentary Push Button - http://a.co/d9lPcVw
* Protoboard for developing, wire/soldering iron for making a permanent setup
* USB Micro cable

Hardware Setup
---------------------
* Solder the header pins to the Development Board
* Lay out the devices on the protoboard
<img src="/docs/proto1.jpg" width="500">

* Wire up your buttons, each button needs to be connected to a pin and on the other side of the button connected to ground (GND). In this example: Up - 13, Down - 12, Back - 14, Enter - 27 
<img src="/docs/proto2.jpg" width="500">

Software Setup
---------------------
* Check the Dev board works by plugging in the micro usb cable, there is usually a demo program which will display lots of shapes on the OLED screen - if nothing turns on inspect the screen, they can be very fragile.
* Install the CP 210x USB/Serial driver for your OS  
  * https://www.silabs.com/products/development-tools/software/usb-to-uart-bridge-vcp-drivers 
* Follow OS specific guides to build the dev environment and flash the wallet
  * [OS X Build Notes](build-osx.md)
  * [Linux Build Notes](build-linux.md)
  * [Windows Build Notes](build-windows.md)
