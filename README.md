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


## Flashing

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

## Support

If you would like to get in touch, please write to us at: [support@joltwallet.com](mailto:support@joltwallet.com)
