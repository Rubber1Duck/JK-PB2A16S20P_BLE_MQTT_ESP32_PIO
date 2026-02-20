#include "publish.h"

// Define the publish task
void publishTask(void *pvParameters)
{
    PublishMessage queue_out;

    while (true)
    {
        while (mqtt_client.state() != MQTT_CONNECTED || !isWifiConnected)
        {
            vTaskDelay(pdMS_TO_TICKS(100)); // Wait until MQTT is connected
        }
        // Receive data from the queue
        if (xQueueReceive(publishQueue, &queue_out, 0) == pdTRUE)
        {
            // DEBUG_PRINTLN("Queue size: " + String(uxQueueMessagesWaiting(publishQueue)));
            // DEBUG_PRINTLN("Publishing to MQTT: " + String(queue_out.topic) + " -> " + String(queue_out.payload));
            //  Call the publish function
            bool success = mqtt_client.publish(queue_out.topic, queue_out.payload);
            if (!success)
            {
                String failMsg = "MQTT publish failed: " + String(queue_out.topic);
                DEBUG_PRINTLN(failMsg);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(30)); // Optional: Add a small delay to prevent task starvation
    }
}

void publish_init()
{
    // Create the publishqueue
    publishQueue = xQueueCreate(150u, sizeof(PublishMessage));
    if (publishQueue == NULL)
    {
        DEBUG_PRINTLN("Failed to create publish queue");
    }
    // Create the publish task
    xTaskCreate(publishTask, "Publish Task", 4096, NULL, 1, NULL);
}