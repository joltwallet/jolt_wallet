Linux Build Instructions and Notes
======================================

Tested on Ubuntu 16.04

Install Dependencies
---------------------
`sudo apt-get install gcc git wget make libncurses-dev flex bison gperf python python-serial cmake`

`wget https://dl.espressif.com/dl/xtensa-esp32-elf-linux64-1.22.0-80-g6c4433a-5.2.0.tar.gz`

Install Compiler
---------------------
`mkdir esp/`

`cd esp/`

`tar -xzf ../xtensa-esp32-elf-linux64-1.22.0-80-g6c4433a-5.2.0.tar.gz`

Add Compiler to PATH
---------------------
Edit `~/.profile` and add

`export PATH=$PATH:$HOME/esp/xtensa-esp32-elf/bin` 

`export IDF_PATH=~/esp/esp-idf`

Install Espressif IoT Development Framework
---------------------
`git clone --recursive https://github.com/espressif/esp-idf.git`

Due to an issue with the latest esp-idf and libwebsockets it is necessary to role back to a previous version.

`cd esp-idf`

`git checkout 8c307`

`git submodule update --init --recursive`

`cd ..`

Install Jolt Wallet
---------------------
`git clone --recursive https://github.com/joltwallet/jolt_wallet.git`

`cd jolt_wallet`

`make menuconfig`

In menuconfig the minimum you need to set (move using key arrows and toggle using spacebar):
* Component_config -> Easy_Input_Config -> Push Buttons -> Set pins
* Component_config -> ESP32-specific -> Main XTAL Frequency -> 26MHz
* Component_config -> Nano ESP32 Library Config -> Enable the Nano Library
* Component_config -> nano_parse -> Build with LWS helpers


`make`

Flash to Device
---------------------

`make flash monitor`
