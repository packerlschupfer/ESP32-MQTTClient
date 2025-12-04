# ESP32MQTTClient - CLAUDE.md

## Overview
Low-level ESP-IDF MQTT client wrapper providing direct access to ESP32's mqtt_client API. This is the underlying transport used by MQTTManager.

## Key Features
- Direct ESP-IDF mqtt_client wrapper
- Topic subscription with callbacks
- SUBACK verification (subscription acknowledgment tracking)
- Last will and testament support
- Reconnection handling
- Message size limits configurable
- IDF 4.x / 5.x compatibility

## Architecture
- Uses `esp_mqtt_client_handle_t` from ESP-IDF
- Subscription records stored in vector with confirmation status
- Callback-based message delivery
- Global event handler registration
- Uses `std::string` for topic/payload (not Arduino String)

## Usage
```cpp
ESP32MQTTClient client;
client.setURI("mqtt://broker.local", "user", "pass");
client.subscribe("topic", [](const std::string& msg) {
    Serial.println(msg.c_str());
});
client.loopStart();

// Check if subscription was acknowledged by broker
if (client.isSubscriptionConfirmed("topic")) {
    // Broker accepted subscription
}
```

## SUBACK Verification
```cpp
// Set callback for subscription acknowledgments
client.setSubscribeAckCallback([](int msgId, const std::string& topic, int grantedQos) {
    if (grantedQos == 0x80) {
        // Broker rejected subscription (ACL rules, etc.)
    }
});

// Query subscription status
bool confirmed = client.isSubscriptionConfirmed("topic");
int qos = client.getSubscriptionQos("topic");  // -1=pending, 0-2=granted, 0x80=rejected
```

## Thread Safety
- ESP-IDF mqtt_client handles internal threading
- Callbacks may be invoked from MQTT task context

## Relationship to MQTTManager
MQTTManager wraps ESP32MQTTClient and adds:
- Singleton pattern
- Event group integration
- Auto-reconnect with backoff
- Higher-level API with Arduino String support

## Build Configuration
```ini
build_flags =
    -DESP32MQTTCLIENT_DEBUG       ; Enable debug logging (MQTTC_LOG_D, MQTTC_LOG_V)
    -DESP32MQTTCLIENT_USE_LOGGER  ; Use Logger class (thread-safe, rate-limited)
```

## Logging
Uses `ESP32MQTTClientLogging.h` with macros:
- `MQTTC_LOG_E/W/I` - Always enabled
- `MQTTC_LOG_D/V` - Only with `ESP32MQTTCLIENT_DEBUG`
- `MQTTC_LOG_PROTO` - Protocol-level debugging
- `MQTTC_LOG_CONN` - Connection state debugging

### Logger Integration
With `-DESP32MQTTCLIENT_USE_LOGGER`:
- Thread-safe logging via mutex
- Rate limiting (150 logs/sec max)
- Per-tag log level control
- Callback context marking
- Uses bundled `logger_submodule`

Without the flag, uses direct ESP_LOG* calls (simpler, no dependencies).
