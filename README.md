<h1 align="center"> Jolt </h1> <br>
<p align="center">
  <a href="https://joltwallet.com/">
    <img alt="Jolt" title="Jolt" src="media/logo.png" width="400">
  </a>
</p>

<p align="center">
  Crypto in your pocket.
</p>

[![Updates](https://pyup.io/repos/github/joltwallet/jolt_wallet/shield.svg)](https://pyup.io/repos/github/joltwallet/jolt_wallet/)

![GitHub repo size](https://img.shields.io/github/repo-size/joltwallet/jolt_wallet)

## Table of Contents

- [Introduction](#introduction)
- [Development](#development)
- [FAQs](#faqs)
- [Roadmap](#roadmap)
- [Contributors](#contributors)
- [Support](#support)

## Introduction

Jolt is a hardware wallet built on [Espressif's IoT Development Framework](https://github.com/espressif/esp-idf) for the ESP32 chip. For
more information about ESP32, see the [ESP32 Website](https://www.espressif.com/en/products/hardware/esp32/overview).

## Development

If you'd like to develop, build, and/or run Jolt please follow the guide that best matches your native platform.

- [OS X Build Notes](docs/build-osx.md)
- [Linux Build Notes](docs/build-linux.md)
- [Windows Build Notes](docs/build-windows.md)

<b>Disclaimer: </b> Just a friendly reminder to please note this project is still under continouus and heavy development, which means things may break or change from time to time.  But that doesn't mean we want it to break! So we ask if you do encounter any issues, to please file them and one of us will try to respond as quickly as possible. Additionally, not all security features are currently enabled, such as hardware encryption or secure boot; without these features, private keys could be extracted with physical access to the device.

## FAQs

Checkout our extensive FAQs for more information - [FAQs](docs/faq.md)

## Roadmap

To see the latest on what we've done, what we're planning, and where we're headed click [here](docs/roadmap.md).

## Known Issues

* App loading only works when set to release mode or stack protection is disabled.


## Environment setup
For all commands, we need to source some environment variables and 
generate the build files so we can take further action.

```
source export.sh  # On initial setup this may error, that's fine
mkdir -p build/ && cd build && cmake .. -G Ninja
```

GNU Make may be used instead of Ninja.  If a build system is not specified,
`Ninja` will be used.


### Setup toolchain and other depdencies

```
# Go into the build directory
cd build/

# Install system dependencies
ninja system_dependencies

# Install toolchain (and system dependencies)
ninja toolchain

# Re-initialize some env vars
cd .. && source export.sh
```

### Vanilla build


```
idf.py build
```


### Compressed build

This will also build the compressed `build/jolt_os.bin.gz` file

```
idf.py compress
```


### Unit Testing

This will build JoltOS with additional testing functionality and override
the default GUI with a unit-testing menu

```
# From the project directory
rm -rf build/  # Make sure the previously generated make/ninja file is gone
idf.py tests
```

## Environment setup (Legacy GNU Make)
[deprecated]

#### Prerequisites
Install the prerequisites specified in the [ESP-IDF docs](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html#get-started-get-prerequisites). The rest of the ESP-iDF steps are handled below.

#### Setup toolchain and other dependencies
Setup the build environement by running the following command. 

```
make install
```

#### Export variables

Finally, run the command below. This must be done in every new bash instance.
```
source export.sh
```

## Flashing

Note: final `sdkconfig.defaults` overrides can be applied via a file 
`sdkconfigs/sdkconfig.personal`. Useful things like device port can be set in
this file.

#### Default JoltOS

```
make flash monitor -j
```

#### Unit Test GUI
The following flashes the on-device debug menu along with some developer commands.

```
make test-menu - j
```

#### Unit Test Functions
The following command will flash JoltOS unit tests and a unit-test menu accessible
via the UART console.

```
make tests -j
```

#### Other hardware targets
JoltOS supports other esp32-based hardware; These generally only differ in supported 
functionality and pinout. To set the target board, set the env var `TARGET_BOARD`
prior to generating `sdkconfig`. For example:
```
TARGET_BOARD=dstike make menuconfig
```
To load the `sdkconfigs/sdkconfig.defaults.dstike` to override some of the definitions in
`sdkconfigs/sdkconfig.defaults`.

## Support

If you would like to get in touch, please write to us at: [support@joltwallet.com](mailto:support@joltwallet.com)
