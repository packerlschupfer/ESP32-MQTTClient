# ESP32MQTTClient Unit Tests

This directory contains unit tests for the ESP32MQTTClient library.

## Test Structure

- `test_esp32mqttclient.cpp` - Basic functionality tests including:
  - Client instantiation
  - Configuration methods
  - Connection state management
  - Publish/Subscribe API tests
  - Logger integration tests
  - Memory management tests

- `test_mqtt_events.cpp` - MQTT event handling tests including:
  - Connect/Disconnect events
  - Data reception events
  - Error event handling
  - Event sequencing
  - Edge cases (null payloads, etc.)

- `test_main.cpp` - Main test runner that executes all test suites

## Running Tests

### With PlatformIO

```bash
# Run tests without custom logger
pio test -e test_esp32

# Run tests with custom logger
pio test -e test_esp32_with_logger

# Run all tests
pio test
```

### Integration with Main Project

To use these tests in your main project:

1. Copy the test files to your project's test directory
2. Include the ESP32MQTTClient library in your `platformio.ini`
3. Add the test configurations from `test/platformio.ini` to your project's `platformio.ini`
4. Run tests using `pio test`

### Test Coverage

The tests cover:
- ✅ Basic instantiation and configuration
- ✅ MQTT URI and credential handling
- ✅ Connection state management
- ✅ Publish/Subscribe API (without actual MQTT broker)
- ✅ Event handling and callbacks
- ✅ Error scenarios
- ✅ Logger integration (both with and without custom logger)
- ✅ Memory management

### Limitations

These unit tests do not require an actual MQTT broker. They test:
- API functionality
- Configuration handling
- Event processing logic
- Error handling
- Logger integration

For integration tests with a real MQTT broker, additional tests would be needed.

### Adding New Tests

To add new tests:
1. Add test functions to the appropriate test file
2. Follow the naming convention: `test_mqtt_<feature>_<scenario>`
3. Use Unity test macros (TEST_ASSERT_*)
4. Add the test to the appropriate RUN_TEST() call in the test runner

### Custom Logger Testing

The tests automatically detect whether `ESP32MQTTCLIENT_USE_LOGGER` is defined:
- When undefined: Tests verify ESP-IDF logging is used
- When defined: Tests verify custom Logger::getInstance() is used

This ensures the library works correctly in both configurations.