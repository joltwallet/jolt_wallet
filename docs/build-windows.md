Windows Build Instructions and Notes
======================================
Tested on Windows 10 Pro Version 1803
This guide is distilled from the official [ESP-IDF Guide](https://esp-idf.readthedocs.io/en/latest/get-started/windows-setup.html). If you run into issues or are curious on the reason behind some of these commands, first check their documentation

# Setup Toolchain
[Download the windows all-in-one toolchain](https://dl.espressif.com/dl/esp32_win32_msys2_environment_and_toolchain-20180110.zip) and unzip the contents to `C:\`

# Install Espressif IoT Development Framework
Double click `C:\msys32\mingw32.exe` and run
```
mkdir -p ~/esp
```
All of our ESP32 project code will go into this folder.

> `mingw32.exe` will be referred to as the terminal in this guide

Download the Espressif IoT Development Framework:
```
cd ~/esp
git clone --recursive https://github.com/espressif/esp-idf.git
cd esp-idf
git checkout 8c307
git submodule update --init --recursive
```

With a text editor, create the file
```
C:/msys32/etc/profile.d/export_idf_path.sh
```
with contents
```
export IDF_PATH="C:/msys32/home/user-name/esp/esp-idf"
```
where `user-name` is your windows username.

Close the terminal and open it again. Verify IDF_PATH is set:
```
printenv IDF_PATH
```

# Install cmake
Enter the following command into terminal:
```
pacman -S mingw-w64-i686-cmake
```

# Install Jolt Wallet
Now its time to clone the Jolt Wallet repo. Type the following commands into the terminal.
```
cd ~/esp
git clone --recursive https://github.com/joltwallet/jolt_wallet.git
```

# Establishing Connection and Configuration
Connect your development board to your computer. Most boards use either a `FTDI` or a `C210x` USB to UART chip. Download and install the appropriate driver for your board. The `Heltec WiFi_Kit_32` uses the C210x chip.

* [FTDI Driver](http://www.ftdichip.com/Drivers/VCP.htm)

* [C210x Driver](https://www.silabs.com/products/development-tools/software/usb-to-uart-bridge-vcp-drivers)

After installing the drivers, we need to find the port your device is connected to. Write click the windows icon on the bottom left of your screen, and select "Device Manager". Under "Ports" you should see something along the lines of
```
Silicon Labs CP210x USB to UART Bridge (COM3)
```
In my case, the port is `COM3`.

Before we can configure the port, we first need to copy over some basic configs. If you are using a `Heltec WiFi_Kit_32` with the suggested hardware setup, the `sdkconfig.heltec_dev` file in `sdkconfigs` is a pretty good start. Copy it to the base directory:
```
cd ~/esp/jolt_wallet
cp sdkconfigs/sdkconfig.heltec_dev sdkconfig
```

To set the port, open up the configuration utility:
```
make menuconfig
```
Then select `Serial flasher config`. Change the `Default serial port` to your serial port; in this example its `COM3`.

# Flash to Device
To build the firmware and flash your device, run
```
make flash monitor
```
from within the `jolt_wallet` folder. 
