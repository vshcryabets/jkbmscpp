# Copilot Instructions for jkbmscpp

## Project Overview
**jkbmscpp** is a C++ library for communicating with Jikong Battery Management Systems (BMS) over Bluetooth Low Energy (BLE). The architecture emphasizes abstraction and pluggability—the BLE communication layer is completely abstracted, allowing any custom BLE implementation to be swapped in.

## Build & Test Commands

### Building the Library
```bash
cd library
mkdir build
cd build
cmake ..
make
```

### Building SimpleBLE Sample
```bash
cd samples/simpleBleSample
mkdir build
cd build
cmake ..
make
./simpleBleSample
```

### Running Library Tests
Tests use Catch2 v3.5.3. Run from the build directory:
```bash
ctest                    # Run all tests
ctest -V                 # Verbose output
ctest -R testJkBmsFrames # Run specific test
```

### Python Build Workflow
The project includes `workflow_build.py` for convenience:
```bash
python3 workflow_build.py --prepare  # Run cmake configure
python3 workflow_build.py --build    # Build the project
python3 workflow_build.py --test     # Run tests
```

## Architecture & Core Concepts

### Dependency Abstraction Pattern
The core design revolves around **Source interface abstraction**:
- `Source` is an abstract interface defined in `JkBmsController.h` that handles all BLE operations (connect, disconnect, send, subscribe/unsubscribe)
- `Controller` (the main library class) operates against this interface, never directly accessing BLE
- Implementations provide concrete `Source` subclasses (e.g., `JkBmsSourceSimpleBleImpl` in samples)
- This allows users to substitute any BLE stack without modifying the library

### Key Classes
- **`Controller`** (`JkBmsController.h/.cpp`) - Main entry point; manages communication lifecycle and device queries
- **`Source`** (`JkBmsController.h`) - Abstract interface for BLE communication
- **`JkBmsFrames.h`** - Protocol data structures: `CellInfoResponse`, `DeviceInfoResponse`, `SettingsResponse`, error enums, response types
- **`JkBmsTools.h`** - Frame parsing utilities and CRC/checksum validation
- **`CppWrappers.h`** - Custom type abstractions (`Expected<T,E>` for error handling, `JkBmsString`, `JkBmsByteBuffer`)

### Communication Flow
1. User creates a concrete `Source` implementation (e.g., SimpleBLE-based)
2. User instantiates `Controller` and calls `start(source)` to connect
3. Controller subscribes to characteristic `0000ffe1-0000-1000-8000-00805f9b34fb` on service `0000ffe0-0000-1000-8000-00805f9b34fb`
4. User calls query methods (e.g., `getDeviceInfo()`, `getCellInfo()`) which return futures
5. Device responds via notifications; Controller parses frames and fulfills futures
6. User calls `end()` to disconnect

### Frame Format
- Magic bytes: `0x55 0xaa`
- Response type (1 byte): SETTINGS (0x01), CELL_INFO (0x02), DEVICE_INFO (0x03)
- Data payload (CRC validated)
- CRC16 checksum at end

## Key Conventions & Patterns

### Error Handling with Expected<T, E>
The codebase uses a custom `Expected<T, E>` type (similar to Rust's Result) for error handling:
```cpp
Expected<DeviceInfoResponse, ControllerError> result = controller->getDeviceInfo().get();
if (result.has_value()) {
    auto info = result.value();
    // use info
} else {
    auto error = result.error();
    // handle error
}
```
Always check `has_value()` before accessing `.value()`.

### Memory & String Handling
- The library supports configurable string representation via CMake option `USE_STD_STRING` (default: ON)
- When ON, uses `std::string` and `std::vector`; when OFF, uses custom `JkBmsString` and `JkBmsBuffer` for embedded environments
- All public APIs use the abstracted types (`JkBmsString`, `JkBmsDataBuffer`) for portability

### Naming Conventions
- Class names are `PascalCase` (e.g., `Controller`, `CellInfoResponse`)
- Enum types are `PascalCase` with `class enum` (e.g., `enum class SourceError : uint8_t`)
- Member variables use `camelCase` with trailing underscore for units (e.g., `batteryVoltage_mV`, `chargeCurrent_mA`)
- Methods are `camelCase` (e.g., `getDeviceInfo()`, `sendCommand()`)

### Response Structures
Data structures in `JkBmsFrames.h` directly mirror protocol bytes:
- Fixed-size arrays for device data (e.g., `uint16_t cellVoltages_mV[32]`)
- Bit masks for enable/warning status (e.g., `uint32_t enabledCellMask`)
- Clear units in field names for all measurements (temperature in raw units, voltage in mV, current in mA, capacity in mAh)

### Futures & Async
- Query methods return futures (e.g., `DeviceInfoFuture`)
- Call `.get()` on the future to wait for response
- Futures are fulfilled by controller's notification callback when device responds

## Testing Strategy
- Tests in `library/test/testJkBmsFrames.cpp` use **Catch2** framework
- Focus on frame parsing validation with known protocol data (see `SAMPLE_CELL_INFO_FRAME` for example)
- Tests verify CRC validation, byte unpacking, and error detection
- Sample includes `responses.txt` with captured device responses for integration validation

## Dependencies
- **CMake** (≥ 3.25)
- **Catch2** (v3.5.3) - fetched via CMake
- **SimpleBLE** (v0.10.4) - fetched via CMake for sample only; not required for library
- **libdbus-cpp-dev** - required on Linux/Armbian for BLE support

## Important Notes
- The library itself has **no external runtime dependencies** beyond the C++ stdlib
- Platform support verified: macOS (Apple Silicon), Linux x86, Armbian
- Maximum packet size is `320` bytes; default `300` bytes
- Service and characteristic UUIDs are hardcoded constants in `Controller`; device must support standard Jikong BMS BLE profile
