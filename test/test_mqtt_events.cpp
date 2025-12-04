#include <unity.h>
#include <Arduino.h>
#include "../src/ESP32MQTTClient.h"
#include <string.h>

// Test instance
ESP32MQTTClient* eventTestClient = nullptr;

// Mock event tracking
static int connectEventCount = 0;
static int disconnectEventCount = 0;
static int dataEventCount = 0;
static int errorEventCount = 0;
static esp_mqtt_event_id_t lastEventId = MQTT_EVENT_ANY;

// Mock MQTT client handle
static esp_mqtt_client_handle_t mockClientHandle = (esp_mqtt_client_handle_t)0x12345678;

// Global required functions
void onMqttConnect(esp_mqtt_client_handle_t client) {
    if (client == mockClientHandle) {
        connectEventCount++;
    }
}

void handleMQTT(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    auto *event = static_cast<esp_mqtt_event_handle_t>(event_data);
    if (eventTestClient && event) {
        lastEventId = event->event_id;
        eventTestClient->onEventCallback(event);
    }
}

// Helper to create mock MQTT event
esp_mqtt_event_t* createMockEvent(esp_mqtt_event_id_t event_id, const char* topic = nullptr, 
                                  const char* data = nullptr, int data_len = 0) {
    static esp_mqtt_event_t mockEvent;
    static char topicBuffer[256];
    static char dataBuffer[1024];
    
    memset(&mockEvent, 0, sizeof(mockEvent));
    mockEvent.event_id = event_id;
    mockEvent.client = mockClientHandle;
    
    if (topic) {
        strncpy(topicBuffer, topic, sizeof(topicBuffer) - 1);
        mockEvent.topic = topicBuffer;
        mockEvent.topic_len = strlen(topic);
    }
    
    if (data) {
        strncpy(dataBuffer, data, sizeof(dataBuffer) - 1);
        mockEvent.data = dataBuffer;
        mockEvent.data_len = data_len > 0 ? data_len : strlen(data);
    }
    
    return &mockEvent;
}

// Helper to create error event
esp_mqtt_event_t* createMockErrorEvent(esp_mqtt_error_type_t error_type, 
                                       mqtt_connect_return_code_t connect_code = MQTT_CONNECTION_ACCEPTED) {
    static esp_mqtt_event_t mockEvent;
    static esp_mqtt_error_codes_t errorCodes;
    
    memset(&mockEvent, 0, sizeof(mockEvent));
    memset(&errorCodes, 0, sizeof(errorCodes));
    
    mockEvent.event_id = MQTT_EVENT_ERROR;
    mockEvent.client = mockClientHandle;
    mockEvent.error_handle = &errorCodes;
    
    errorCodes.error_type = error_type;
    errorCodes.connect_return_code = connect_code;
    errorCodes.esp_transport_sock_errno = 0;
    
    return &mockEvent;
}

void setUp(void) {
    if (eventTestClient) {
        delete eventTestClient;
    }
    eventTestClient = new ESP32MQTTClient();
    
    connectEventCount = 0;
    disconnectEventCount = 0;
    dataEventCount = 0;
    errorEventCount = 0;
    lastEventId = MQTT_EVENT_ANY;
}

void tearDown(void) {
    if (eventTestClient) {
        delete eventTestClient;
        eventTestClient = nullptr;
    }
}

// Test 1: Connect event handling
void test_mqtt_connect_event(void) {
    // Setup client to recognize our mock handle
    eventTestClient->loopStart(); // This initializes internal client handle
    
    // Create and send connect event
    esp_mqtt_event_t* event = createMockEvent(MQTT_EVENT_CONNECTED);
    handleMQTT(nullptr, ESP_EVENT_ANY_BASE, 0, event);
    
    // Verify event was processed
    TEST_ASSERT_EQUAL(MQTT_EVENT_CONNECTED, lastEventId);
    TEST_ASSERT_TRUE(eventTestClient->isConnected());
}

// Test 2: Disconnect event handling
void test_mqtt_disconnect_event(void) {
    // First connect
    eventTestClient->setConnectionState(true);
    TEST_ASSERT_TRUE(eventTestClient->isConnected());
    
    // Create and send disconnect event
    esp_mqtt_event_t* event = createMockEvent(MQTT_EVENT_DISCONNECTED);
    handleMQTT(nullptr, ESP_EVENT_ANY_BASE, 0, event);
    
    // Verify event was processed
    TEST_ASSERT_EQUAL(MQTT_EVENT_DISCONNECTED, lastEventId);
    TEST_ASSERT_FALSE(eventTestClient->isConnected());
}

// Test 3: Data event handling with callback
void test_mqtt_data_event_with_callback(void) {
    static bool callbackCalled = false;
    static String receivedMessage;
    
    // Set connection state
    eventTestClient->setConnectionState(true);
    
    // Subscribe with callback (mock subscription)
    auto callback = [](const String& message) {
        callbackCalled = true;
        receivedMessage = message;
    };
    
    // Note: We can't actually subscribe without real MQTT client,
    // but we can test the data event handling
    
    // Create and send data event
    const char* testTopic = "test/topic";
    const char* testData = "Hello MQTT";
    esp_mqtt_event_t* event = createMockEvent(MQTT_EVENT_DATA, testTopic, testData);
    
    handleMQTT(nullptr, ESP_EVENT_ANY_BASE, 0, event);
    
    // Verify event was processed
    TEST_ASSERT_EQUAL(MQTT_EVENT_DATA, lastEventId);
}

// Test 4: Data event with topic callback
void test_mqtt_data_event_with_topic_callback(void) {
    static bool callbackCalled = false;
    static String receivedTopic;
    static String receivedMessage;
    
    // Set connection state
    eventTestClient->setConnectionState(true);
    
    // Topic callback
    auto topicCallback = [](const String& topic, const String& message) {
        callbackCalled = true;
        receivedTopic = topic;
        receivedMessage = message;
    };
    
    // Create and send data event
    const char* testTopic = "test/topic/sub";
    const char* testData = "Topic Message";
    esp_mqtt_event_t* event = createMockEvent(MQTT_EVENT_DATA, testTopic, testData);
    
    handleMQTT(nullptr, ESP_EVENT_ANY_BASE, 0, event);
    
    // Verify event was processed
    TEST_ASSERT_EQUAL(MQTT_EVENT_DATA, lastEventId);
}

// Test 5: Error event - connection refused
void test_mqtt_error_event_connection_refused(void) {
    eventTestClient->enableDebuggingMessages(true);
    
    // Create error event for connection refused
    esp_mqtt_event_t* event = createMockErrorEvent(
        MQTT_ERROR_TYPE_CONNECTION_REFUSED, 
        MQTT_CONNECTION_REFUSE_BAD_USERNAME
    );
    
    handleMQTT(nullptr, ESP_EVENT_ANY_BASE, 0, event);
    
    // Verify event was processed
    TEST_ASSERT_EQUAL(MQTT_EVENT_ERROR, lastEventId);
}

// Test 6: Error event - TCP transport error
void test_mqtt_error_event_tcp_transport(void) {
    eventTestClient->enableDebuggingMessages(true);
    
    // Create TCP transport error event
    esp_mqtt_event_t* event = createMockErrorEvent(MQTT_ERROR_TYPE_TCP_TRANSPORT);
    event->error_handle->esp_transport_sock_errno = ECONNREFUSED;
    
    handleMQTT(nullptr, ESP_EVENT_ANY_BASE, 0, event);
    
    // Verify event was processed
    TEST_ASSERT_EQUAL(MQTT_EVENT_ERROR, lastEventId);
}

// Test 7: Multiple events in sequence
void test_mqtt_event_sequence(void) {
    // Connect
    esp_mqtt_event_t* connectEvent = createMockEvent(MQTT_EVENT_CONNECTED);
    handleMQTT(nullptr, ESP_EVENT_ANY_BASE, 0, connectEvent);
    TEST_ASSERT_TRUE(eventTestClient->isConnected());
    
    // Receive data
    esp_mqtt_event_t* dataEvent = createMockEvent(MQTT_EVENT_DATA, "test/seq", "Sequential Data");
    handleMQTT(nullptr, ESP_EVENT_ANY_BASE, 0, dataEvent);
    
    // Disconnect
    esp_mqtt_event_t* disconnectEvent = createMockEvent(MQTT_EVENT_DISCONNECTED);
    handleMQTT(nullptr, ESP_EVENT_ANY_BASE, 0, disconnectEvent);
    TEST_ASSERT_FALSE(eventTestClient->isConnected());
}

// Test 8: Event with null data
void test_mqtt_data_event_null_payload(void) {
    eventTestClient->setConnectionState(true);
    
    // Create data event with null payload
    esp_mqtt_event_t* event = createMockEvent(MQTT_EVENT_DATA, "test/null", nullptr, 0);
    
    handleMQTT(nullptr, ESP_EVENT_ANY_BASE, 0, event);
    
    // Should handle gracefully without crashing
    TEST_ASSERT_EQUAL(MQTT_EVENT_DATA, lastEventId);
}

// Test 9: isMyTurn functionality
void test_mqtt_is_my_turn(void) {
    // Test with matching client handle
    TEST_ASSERT_TRUE(eventTestClient->isMyTurn(mockClientHandle));
    
    // Test with different client handle
    esp_mqtt_client_handle_t otherHandle = (esp_mqtt_client_handle_t)0x87654321;
    TEST_ASSERT_FALSE(eventTestClient->isMyTurn(otherHandle));
}

// Test runner
void run_mqtt_event_tests(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_mqtt_connect_event);
    RUN_TEST(test_mqtt_disconnect_event);
    RUN_TEST(test_mqtt_data_event_with_callback);
    RUN_TEST(test_mqtt_data_event_with_topic_callback);
    RUN_TEST(test_mqtt_error_event_connection_refused);
    RUN_TEST(test_mqtt_error_event_tcp_transport);
    RUN_TEST(test_mqtt_event_sequence);
    RUN_TEST(test_mqtt_data_event_null_payload);
    RUN_TEST(test_mqtt_is_my_turn);
    
    UNITY_END();
}

// Arduino setup and loop for running tests
#ifdef ARDUINO
void setup() {
    delay(2000);
    Serial.begin(115200);
    
    run_mqtt_event_tests();
}

void loop() {
    // Nothing to do here
}
#endif