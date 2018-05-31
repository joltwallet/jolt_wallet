Frequently Asked Questions
======================================

What is Jolt Wallet?
---------------------
Jolt is a cryptocurrency hardware wallet. A hardware wallet is a device that securely stores and uses your private keys to sign transactions and messages. Designed with Nano in mind, Jolt acts in a similar way to other light wallets where it communicates with a server to get network information and then performs all the cryptographic processing on-device. Network data is always verified with cryptographic proofs to keep your funds safe, even in the event of a hacked server. Your private keys never leave the device and are encrypted with a pin to keep unauthorized users out.

Why would I want to use a Jolt Wallet instead of another hardware wallet?
--------------------------------------------------------------------------
We recognise there are some excellent, well established hardware wallets already on the market; however, they are not accessible to everyone due to their high costs. Furthermore, we believe that current hardware wallets take up too much time and effort to be used for everyday transactions. Jolt is based on the inexpensive ESP32 chipset, providing a rich feature set like onboard wifi and bluetooth. These wireless communications allows Jolt to directly communicate with servers and cryptocurrency networks rather than requiring a desktop/laptop. It has also been designed from the ground up to work with Nano and take advantage of feeless and near instant transactions.
Jolt has both open source software and will have reference open source hardware which allows for further innovation and development, we look forward to releasing some additional products.

How do I get a Jolt Wallet?
----------------------------
Jolt is still in early development and we plan to release a fully formed product in the future, in the meantime for the adventurous there are a number of ways you can build your own Jolt Wallet:
* Heltec WiFi_Kit_32 Development Board + 4 buttons 
  * These are easily and cheaply available off Amazon under other brand names such as MakerHawk or MakerFocus, we have used these boards to develop the software. Besides the development board, it is necessary to add only 4 buttons to allow you to navigate the system which will require some simple wiring. No other components are required. Please see are online guide for more information, links and a guide to building and flashing.
* Prototype your own
  * ESP32 + OLED screen + Buttons + USB/Serial + Power Source
  * Jolt was intentionally designed to use readily available, hobbyist-friendly parts.
* Sign up for early access to custom PCB
  * We are in the process of designing and manufacturing our own PCBs, sign up to our mailing list to be in for a chance of getting hold of the first generation
* Attend our workshops (dates/locations to be announced)
  * We plan to hold some workshops where you can make your own hardware wallet (using one of our first gen PCBs), learn how to solder, checkout some 3d printers and have a beer. Sign up to our mailing list for more information.
* Wait for our release
  * We are in the process of designing and manufacturing a commercial form of Jolt which will be for sale in the near future (please be aware that asking ‘when Jolt?’ will not speed up the manufacturing process - it will be ready when it is ready…)

Why have you open sourced the software?
----------------------------------------
We want people to be able to review, audit, contribute and innovate, an embedded light wallet on a cheap multi featured chip has a lot of potential use beyond a hardware wallet - check out our Labs page for inspiration.
We are in the process of developing a consumer version of Jolt which will be sold for profit to fund further development and expansion.

What are your links to Nano?
-----------------------------
The Jolt Wallet Team is a spin off from Nano Core, we now operate independently however continue to maintain close links. We have been working on this project for a number of months and have a lot of experience in the Nano protocol, using Nano in applications but also embedded development.

How have you developed Jolt Wallet?
------------------------------------
We have developed Jolt using the official Espressif-IDF toolchain, the code is completely written in C. While there are certain ESP32 specific elements (particularly around internal encryption) everything else is based on generic well established open source libraries and so would be easily portable to other platforms.

What security features does the Jolt Wallet have?
--------------------------------------------------
The ESP32 chipset has built in AES-256 flash encryption. This means that even if someone was probing the circuitry with sophisticated equipment, they wouldn’t be able to get a single bit of meaningful data. On top of this, the master seed is also encrypted via the user’s pin while in RAM to prevent data remanence attacks. After 3 minutes of inactivity, the master seed is wiped from RAM. After 10 pin entry failures, the master seed is wiped from storage and the device performs a factory reset.

What Nano security features does the Jolt Wallet have?
-------------------------------------------------------
When performing any Nano transaction, Jolt requests the full head block of your Nano account from the server. Ever since State Blocks were released, an account’s entire state is encoded in the head block. Using the head block, we can locally compute the exact values needed for the transaction we are creating. If a hacker sends erroneous account information, the resulting signed transaction would be rejected by the Nano network and no funds would ever be at risk. Transaction data like amount and destination are presented on Jolt’s screen for user confirmation.

How can I follow development?
------------------------------
* Our Website (https://www.joltwallet.com/)
* Github (https://github.com/joltwallet)
* Sign up to our mailing list for updates (https://www.joltwallet.com/)
* Regular posts to Reddit ([/r/nanocurrency](https://www.reddit.com/r/nanocurrency/))
* Medium (https://medium.com/@joltwallet)
* Twitter (https://twitter.com/JoltWallet)

What social media do you use?
------------------------------
* We only use [Twitter](https://twitter.com/JoltWallet) and post on the [/r/nanocurrency](https://www.reddit.com/r/nanocurrency/) reddit and our [Medium Account](https://medium.com/@joltwallet)
* We do not use Facebook/Instagram/SnapChat etc
* We will never run a reward system where you send <crypto> and get back more - these a scams/phishing scams

Can I run my own server/backend?
---------------------------------
It’s easy to run your own server (either for personal use or for the community), all that is required is a system running a Nano Node with its RPC open (we recommend that you don’t have any wallets/accounts on the node) and then to run our raicast python server which will act as an interface for the hardware wallets via websockets.

Will you support other cryptocurrencies?
-----------------------------------------
Yes, we plan to add other cryptocurrencies in the future (however they won’t be necessarily as feature rich as Nano).

Approximately how much will the future releases cost?
------------------------------------------------------
We are aiming for something that will cost less than $35.

Where did this come from?
--------------------------
The first prototype for a Nano based embedded hardware wallet started in December 2017 and was based on using an e-ink screen and a raspberry pi zero running a version of the RetroXRBWallet. This version was overkill and we recognized that we could use a simpler, low cost, low powered microprocessor instead. Work started on the ESP32 version at the end of January 2018 and we have already worked through a number of iterations. 

What is NanoRay?
-----------------
NanoRay was the original project code name however due to other available hardware wallets its not appropriate to continue using that name. We are in the process of changing any reference of NanoRay to Jolt.

Comparison with other hardware wallets
----------------------------------------

| Feature | Jolt | Trezor | Ledger Nano S |
|---------|------|--------|---------------|
| Microcontroller | ESP-WROOM-32 | STM32F205VET6 | STM32F042K + ST31H320 |
| Price | $3.95 | $10.25 | $1.44 + $??? |
| Frequency | 240MHz | 120MHz | 48MHz + 28MHz |
| RAM | 520KB | 64KB | 6KB + 10KB |
| Program Memory Stats | 4096KB | 512KB | 32KB + 320KB |
| WiFi 802.11BGN | Yes | No | No |
| Bluetooth | Yes | No | No |
| Performance | 600 DMIPS | 150DMIPS | ??? |
| Screen | 128x64 OLED | 128x64 OLED | 128x64 OLED |
| Open Source | Yes | Yes | Mostly Not |
| microSD | Yes | No | No |

