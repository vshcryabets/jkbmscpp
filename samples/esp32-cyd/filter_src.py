Import("env")
from os.path import join, realpath

build_flags = env.GetProjectOption("build_flags", [])

sources = ["+<*>"]
# print("Build flags:", build_flags)
if "-DENABLE_ArduinoBLE" not in build_flags:
    print("ArduinoBLE is disabled. Excluding BLE source files...")
    sources.append("-<ble/BleScannerBLEArduinoImpl.cpp>")
    sources.append("-<ble/JkBmsSourceBLEArduinoImpl.cpp>")

if "-DENABLE_NimBLE" not in build_flags:
    print("NimBLE is disabled. Excluding BLE source files...")
    sources.append("-<ble/BleScannerNimBLEImpl.cpp>")

env.Replace(SRC_FILTER=sources)