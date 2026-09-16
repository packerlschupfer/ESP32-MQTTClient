# Changelog

All notable changes to this project will be documented in this file.

## [Unreleased]

### Added
- `setNetworkTimeout(int timeoutMs)` sets esp-mqtt's network timeout
  (`network.timeout_ms` on IDF 5.x, `network_timeout_ms` on IDF 4.x), which was
  previously left at the 10 s default. Besides bounding normal network operations,
  that value also governs how long `esp_mqtt_client_stop()` blocks while tearing the
  client down - a measured teardown took ~10.4 s, which overran the 10 s window
  espota allows a device to answer an OTA invitation. Must be called before the
  first `loopStart()`, since esp-mqtt copies the config at `esp_mqtt_client_init()`.

## [0.1.0] - 2025-12-04

### Added
- Initial public release
- ESP32 MQTT client wrapper
- Thread-safe logging support
- Optional logger integration via ESP32MQTTClient_USE_LOGGER define
- Async MQTT operations
- Auto-reconnect support

### Notes
- Production-tested in industrial IoT application
- Previous internal versions not publicly released
- Reset to v0.1.0 for clean public release
