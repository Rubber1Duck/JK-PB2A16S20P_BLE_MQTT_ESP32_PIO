#include "publish.h"
uint8_t maxUsedQueueSize = 0;
uint8_t oldMaxUsedQueueSize = 0;

// Define the publish task
void publishTask(void *pvParameters) {
    PublishMessage queue_out;

    while (true) {
        while (mqtt_client.state() != MQTT_CONNECTED || !isWifiConnected) {
            vTaskDelay(pdMS_TO_TICKS(100)); // Wait until MQTT is connected
        }
        // Receive data from the queue
        if (xQueueReceive(publishQueue, &queue_out, 0) == pdTRUE) {
            uint8_t currentQueueSize = uxQueueMessagesWaiting(publishQueue);
            maxUsedQueueSize = max(maxUsedQueueSize, currentQueueSize); // to track max used queue size for debugging purposes, this musst be under 150
            if (maxUsedQueueSize > oldMaxUsedQueueSize) {
                oldMaxUsedQueueSize = maxUsedQueueSize;
                setState("maxpubqueue", String(maxUsedQueueSize), true);
            }
            
            //  Call the publish function
            bool success = mqtt_client.publish(queue_out.topic, queue_out.payload);
            if (!success) {
                String failMsg = "MQTT publish failed: " + String(queue_out.topic);
                DEBUG_PRINTLN(failMsg);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(corePubDelay)); // time between publish attempts, can be adjust via MQTT, default is 50ms, which means max 20 publishes per second, adjust if you have a lot of messages to publish and the queue is filling up, but be careful with too low values as it can cause stability issues with the MQTT client  
    }
}

void publish_init() {
    // Create the publishqueue
    publishQueue = xQueueCreate(150u, sizeof(PublishMessage)); // Queue can hold 150 messages, adjust as needed
    if (publishQueue == NULL) {
        DEBUG_PRINTLN("Failed to create publish queue! Restarting ESP"); //without this, the system cannot function properly, so we restart to try again
        ESP.restart(); // Restart if queue creation fails
    }
    // Create the publish task
    xTaskCreate(publishTask, "Publish Task", 4096, NULL, 1, NULL);
}