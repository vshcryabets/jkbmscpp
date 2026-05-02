# jkbmscpp

A C++ library designed for communication and interaction with **Jikong Battery Management Systems (BMS)**.

This library is released under the Apache 2.0 License. 
However, SimpleBLE sample licensed under the GPLv3 license.

Protocol descritpion was taken from the project https://github.com/syssi/esphome-jk-bms/.

## Overview
jkbmscpp provides a streamlined interface to read data and manage Jikong BMS hardware. 
Currently, the library supports communication exclusively over Bluetooth Low Energy (BLE).

## Key Features
**Jikong BMS Integration:** Built specifically to handle Jikong BMS data structures 
and communication protocols.

**Interchangeable BLE Architecture:** The BLE communication layer is completely abstracted 
and pluggable. You are not locked into a single Bluetooth stack.

**SimpleBLE Support:** Out-of-the-box support using SimpleBLE, but it can be easily swapped 
for any other custom BLE implementation depending on your framework or RTOS requirements.

## Compatibility & Testing
The library has been successfully verified and tested using the SimpleBLE implementation 
across the following platforms and architectures:

- macOS (Apple Silicon)

- Linux x86

- Armbian (Specifically tested on Orange Pi Zero 2W)

## Build instructions

### Armbian

Some packages required to build sample:

```
sudo apt install cmake gcc libdbus-cpp-dev
```
