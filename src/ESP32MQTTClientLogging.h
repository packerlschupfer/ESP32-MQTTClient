#ifndef ESP32MQTTCLIENT_LOGGING_H
#define ESP32MQTTCLIENT_LOGGING_H

#include <esp_log.h>

// Compile-time option to use Logger class (thread-safe, rate-limited)
// Define ESP32MQTTCLIENT_USE_LOGGER to enable Logger integration
// Otherwise, use direct ESP_LOG* calls

#define MQTTC_LOG_TAG "MQTTC"

#ifdef ESP32MQTTCLIENT_USE_LOGGER
    // Use Logger class from logger_submodule
    #include "../logger_submodule/src/Logger.h"

    // Thread-safe, rate-limited logging via Logger singleton
    #define MQTTC_LOG_E(...) getLogger().log(ESP_LOG_ERROR, MQTTC_LOG_TAG, __VA_ARGS__)
    #define MQTTC_LOG_W(...) getLogger().log(ESP_LOG_WARN, MQTTC_LOG_TAG, __VA_ARGS__)
    #define MQTTC_LOG_I(...) getLogger().log(ESP_LOG_INFO, MQTTC_LOG_TAG, __VA_ARGS__)

    #ifdef ESP32MQTTCLIENT_DEBUG
        #define MQTTC_LOG_D(...) getLogger().log(ESP_LOG_DEBUG, MQTTC_LOG_TAG, __VA_ARGS__)
        #define MQTTC_LOG_V(...) getLogger().log(ESP_LOG_VERBOSE, MQTTC_LOG_TAG, __VA_ARGS__)
    #else
        #define MQTTC_LOG_D(...) ((void)0)
        #define MQTTC_LOG_V(...) ((void)0)
    #endif

#else
    // Direct ESP-IDF logging (no thread safety or rate limiting)
    #define MQTTC_LOG_E(...) ESP_LOGE(MQTTC_LOG_TAG, __VA_ARGS__)
    #define MQTTC_LOG_W(...) ESP_LOGW(MQTTC_LOG_TAG, __VA_ARGS__)
    #define MQTTC_LOG_I(...) ESP_LOGI(MQTTC_LOG_TAG, __VA_ARGS__)

    #ifdef ESP32MQTTCLIENT_DEBUG
        #define MQTTC_LOG_D(...) ESP_LOGD(MQTTC_LOG_TAG, __VA_ARGS__)
        #define MQTTC_LOG_V(...) ESP_LOGV(MQTTC_LOG_TAG, __VA_ARGS__)
    #else
        #define MQTTC_LOG_D(...) ((void)0)
        #define MQTTC_LOG_V(...) ((void)0)
    #endif
#endif

// Feature-specific debug flags
#ifdef ESP32MQTTCLIENT_DEBUG
    #define MQTTC_DEBUG_PROTOCOL
    #define MQTTC_DEBUG_CONNECTION
#endif

// Protocol debugging helper (subscribe/publish/data)
#ifdef MQTTC_DEBUG_PROTOCOL
    #define MQTTC_LOG_PROTO(...) MQTTC_LOG_D(__VA_ARGS__)
#else
    #define MQTTC_LOG_PROTO(...) ((void)0)
#endif

// Connection debugging helper
#ifdef MQTTC_DEBUG_CONNECTION
    #define MQTTC_LOG_CONN(...) MQTTC_LOG_D(__VA_ARGS__)
#else
    #define MQTTC_LOG_CONN(...) ((void)0)
#endif

#endif // ESP32MQTTCLIENT_LOGGING_H
