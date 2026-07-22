/**
 * @file test_main.cpp
 * @brief Main test runner for ESP32MQTTClient library
 * 
 * This file provides the main entry point for running all ESP32MQTTClient tests.
 * It can be used in both PlatformIO native tests and Arduino environments.
 */

#include <unity.h>

// Forward declarations for test suites
void run_mqtt_client_tests(void);
void run_mqtt_event_tests(void);

// Main test runner
void run_all_tests() {
    // Run all test suites
    run_mqtt_client_tests();
    run_mqtt_event_tests();
}

// PlatformIO native test entry point
#ifndef ARDUINO
int main(int argc, char **argv) {
    UNITY_BEGIN();
    run_all_tests();
    return UNITY_END();
}
#endif

// Arduino entry points
#ifdef ARDUINO
void setup() {
    delay(2000); // Wait for serial monitor
    Serial.begin(115200);
    Serial.println("Starting ESP32MQTTClient Tests...");
    
    run_all_tests();
}

void loop() {
    // Nothing to do in loop
}
#endif