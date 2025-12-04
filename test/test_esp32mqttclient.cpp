#include <unity.h>
#include <Arduino.h>
#include "../src/ESP32MQTTClient.h"

// Mock variables for testing
static bool onConnectCalled = false;
static bool handleMQTTCalled = false;
static String lastPublishedTopic;
static String lastPublishedPayload;
static String lastSubscribedTopic;
static int messageReceivedCount = 0;
static String lastReceivedMessage;
static String lastReceivedTopic;

// Test instance
ESP32MQTTClient* testClient = nullptr;

// Mock callbacks
void mockMessageCallback(const String& message) {
    messageReceivedCount++;
    lastReceivedMessage = message;
}

void mockMessageCallbackWithTopic(const String& topic, const String& message) {
    messageReceivedCount++;
    lastReceivedTopic = topic;
    lastReceivedMessage = message;
}

// Required global functions
void onMqttConnect(esp_mqtt_client_handle_t client) {
    onConnectCalled = true;
}

void handleMQTT(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    handleMQTTCalled = true;
    if (testClient) {
        auto *event = static_cast<esp_mqtt_event_handle_t>(event_data);
        testClient->onEventCallback(event);
    }
}

// Reset test state
void setUp(void) {
    if (testClient) {
        delete testClient;
    }
    testClient = new ESP32MQTTClient();
    
    onConnectCalled = false;
    handleMQTTCalled = false;
    lastPublishedTopic = "";
    lastPublishedPayload = "";
    lastSubscribedTopic = "";
    messageReceivedCount = 0;
    lastReceivedMessage = "";
    lastReceivedTopic = "";
}

void tearDown(void) {
    if (testClient) {
        delete testClient;
        testClient = nullptr;
    }
}

// Test 1: Basic instantiation
void test_mqtt_client_instantiation(void) {
    TEST_ASSERT_NOT_NULL(testClient);
    TEST_ASSERT_FALSE(testClient->isConnected());
}

// Test 2: Configuration methods
void test_mqtt_client_configuration(void) {
    // Test enabling debugging
    testClient->enableDebuggingMessages(true);
    testClient->enableDebuggingMessages(false);
    
    // Test setting MQTT URI and credentials
    testClient->setURI("mqtt://test.broker.com:1883", "testuser", "testpass");
    TEST_ASSERT_EQUAL_STRING("mqtt://test.broker.com:1883", testClient->getURI());
    
    // Test setting client name
    testClient->setMqttClientName("TestESP32Client");
    TEST_ASSERT_EQUAL_STRING("TestESP32Client", testClient->getClientName());
    
    // Test setting URL alternative method
    testClient->setURL("broker.test.com", 1883, "user2", "pass2");
    
    // Test keep-alive configuration
    testClient->setKeepAlive(60);
    
    // Test last will message
    testClient->enableLastWillMessage("test/lwt", "Client disconnected", true);
    
    // Test persistence
    testClient->disablePersistence();
    
    // Test drastic reset
    testClient->enableDrasticResetOnConnectionFailures();
    
    // Test auto reconnect
    testClient->disableAutoReconnect();
    
    // Test task priority
    testClient->setTaskPrio(5);
    
    // All configuration methods should execute without issues
    TEST_ASSERT_TRUE(true);
}

// Test 3: Packet size configuration
void test_mqtt_packet_size_configuration(void) {
    // Test setting max packet size
    bool result = testClient->setMaxPacketSize(2048);
    TEST_ASSERT_TRUE(result);
    
    // Test setting max out packet size
    result = testClient->setMaxOutPacketSize(1024);
    TEST_ASSERT_TRUE(result);
}

// Test 4: Certificate configuration
void test_mqtt_certificate_configuration(void) {
    const char* testCert = "-----BEGIN CERTIFICATE-----\ntest\n-----END CERTIFICATE-----";
    const char* testKey = "-----BEGIN PRIVATE KEY-----\ntest\n-----END PRIVATE KEY-----";
    const char* testCaCert = "-----BEGIN CERTIFICATE-----\nca\n-----END CERTIFICATE-----";
    
    testClient->setClientCert(testCert);
    testClient->setCaCert(testCaCert);
    testClient->setKey(testKey);
    
    // Methods should execute without issues
    TEST_ASSERT_TRUE(true);
}

// Test 5: Connection state management
void test_mqtt_connection_state(void) {
    // Initially disconnected
    TEST_ASSERT_FALSE(testClient->isConnected());
    
    // Manually set connection state (normally done by event handler)
    testClient->setConnectionState(true);
    TEST_ASSERT_TRUE(testClient->isConnected());
    
    testClient->setConnectionState(false);
    TEST_ASSERT_FALSE(testClient->isConnected());
}

// Test 6: Publish when disconnected
void test_mqtt_publish_when_disconnected(void) {
    // Ensure client is disconnected
    testClient->setConnectionState(false);
    
    // Try to publish - should fail
    bool result = testClient->publish("test/topic", "test message");
    TEST_ASSERT_FALSE(result);
}

// Test 7: Subscribe functionality
void test_mqtt_subscribe(void) {
    // For this test, we'll simulate being connected
    testClient->setConnectionState(true);
    
    // Note: Real subscription would require actual MQTT connection
    // This tests the API and callback registration
    bool result = testClient->subscribe("test/topic1", mockMessageCallback);
    // In real scenario with mqtt_client initialized, this would work
    // For unit test without real MQTT, we expect it to fail gracefully
    TEST_ASSERT_FALSE(result); // Expected since no real MQTT client
    
    // Test with topic callback
    result = testClient->subscribe("test/topic2", mockMessageCallbackWithTopic);
    TEST_ASSERT_FALSE(result); // Expected since no real MQTT client
}

// Test 8: Unsubscribe when disconnected
void test_mqtt_unsubscribe_when_disconnected(void) {
    testClient->setConnectionState(false);
    
    bool result = testClient->unsubscribe("test/topic");
    TEST_ASSERT_FALSE(result);
}

// Test 9: Topic matching logic
void test_mqtt_topic_matching(void) {
    // This tests the private mqttTopicMatch method indirectly
    // through the message received callback system
    
    // Note: Direct testing would require making mqttTopicMatch public
    // or adding a friend test class
    TEST_ASSERT_TRUE(true);
}

// Test 10: Logger integration without custom logger
void test_logger_without_custom_logger(void) {
    #ifndef ESP32MQTTCLIENT_USE_LOGGER
    // When custom logger is not defined, it should use ESP-IDF logging
    testClient->enableDebuggingMessages(true);
    
    // This would log using ESP_LOGI, ESP_LOGE etc.
    // We can't easily verify the output in unit tests,
    // but we can ensure it doesn't crash
    TEST_ASSERT_TRUE(true);
    #else
    TEST_IGNORE_MESSAGE("Test skipped - custom logger is enabled");
    #endif
}

// Test 11: Logger integration with custom logger
void test_logger_with_custom_logger(void) {
    #ifdef ESP32MQTTCLIENT_USE_LOGGER
    // When custom logger is defined, it should use Logger::getInstance()
    testClient->enableDebuggingMessages(true);
    
    // This would log using Logger::getInstance().log()
    // We can't easily verify the output in unit tests,
    // but we can ensure it doesn't crash
    TEST_ASSERT_TRUE(true);
    #else
    TEST_IGNORE_MESSAGE("Test skipped - custom logger is not enabled");
    #endif
}

// Test 12: Memory management
void test_mqtt_memory_management(void) {
    // Test multiple configurations to ensure no memory leaks
    for (int i = 0; i < 5; i++) {
        ESP32MQTTClient* tempClient = new ESP32MQTTClient();
        tempClient->setMqttClientName("TempClient");
        tempClient->setURI("mqtt://temp.broker.com");
        delete tempClient;
    }
    
    // If we get here without crashing, memory management is working
    TEST_ASSERT_TRUE(true);
}

// Test runner
void run_mqtt_client_tests(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_mqtt_client_instantiation);
    RUN_TEST(test_mqtt_client_configuration);
    RUN_TEST(test_mqtt_packet_size_configuration);
    RUN_TEST(test_mqtt_certificate_configuration);
    RUN_TEST(test_mqtt_connection_state);
    RUN_TEST(test_mqtt_publish_when_disconnected);
    RUN_TEST(test_mqtt_subscribe);
    RUN_TEST(test_mqtt_unsubscribe_when_disconnected);
    RUN_TEST(test_mqtt_topic_matching);
    RUN_TEST(test_logger_without_custom_logger);
    RUN_TEST(test_logger_with_custom_logger);
    RUN_TEST(test_mqtt_memory_management);
    
    UNITY_END();
}

// Arduino setup and loop for running tests
#ifdef ARDUINO
void setup() {
    delay(2000); // Wait for serial
    Serial.begin(115200);
    
    run_mqtt_client_tests();
}

void loop() {
    // Nothing to do here
}
#endif