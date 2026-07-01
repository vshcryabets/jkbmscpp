# jkbmscpp

A C++ library designed for communication and interaction with **Jikong Battery Management Systems (BMS)**.

This library is released under the Apache 2.0 License. 
However, SimpleBLE sample licensed under the GPLv3 license.

Protocol description was taken from the project https://github.com/syssi/esphome-jk-bms/.

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

- Armbian (Specifically tested on Orange Pi Zero 2W/3)

## Build instructions

### Armbian

Some packages required to build sample:

```
sudo apt install cmake gcc libdbus-cpp-dev
```

### Building samples/simpleBleSample with CMake

To build the `samples/simpleBleSample` directory using CMake, follow these steps:

1. **Navigate to the Sample Directory:**

   ```sh
   cd samples/simpleBleSample
   ```

2. **Create a Build Directory:**

   It's recommended to create a separate build directory to keep the source code clean.

   ```sh
   mkdir build
   cd build
   ```

3. **Run CMake:**

   Configure the project using CMake. This will generate the necessary build files.

   ```sh
   cmake ..
   ```

4. **Build the Project:**

   Compile the project using your preferred build system, such as `make`.

   ```sh
   make
   ```

5. **Run the Executable:**

   After building, you can run the executable to test the SimpleBLE sample.

   ```sh
   ./simpleBleSample
   ```

This will build and execute the `simpleBleSample` application using CMake.
