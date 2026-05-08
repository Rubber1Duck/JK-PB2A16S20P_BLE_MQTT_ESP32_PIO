#include "publish.h"

QueueHandle_t publishQueue = NULL;
QueueHandle_t rawPublishQueue = NULL;

uint8_t maxUsedQueueSize = 0;
uint8_t oldMaxUsedQueueSize = 0;

static uint8_t rawDataPool[RAWDATA_POOL_SLOT_COUNT][RAWDATA_POOL_SLOT_SIZE];
static QueueHandle_t rawDataFreeSlots = NULL;
static bool rawInfraInitialized = false;
static std::mutex rawInitMutex;

bool rawDataPoolAllocSlot(uint16_t *slotIndex, TickType_t waitTicks)
{
    if (slotIndex == nullptr || rawDataFreeSlots == NULL)
    {
        return false;
    }
    return xQueueReceive(rawDataFreeSlots, slotIndex, waitTicks) == pdTRUE;
}

void rawDataPoolFreeSlot(uint16_t slotIndex)
{
    if (rawDataFreeSlots == NULL || slotIndex >= RAWDATA_POOL_SLOT_COUNT)
    {
        return;
    }
    xQueueSend(rawDataFreeSlots, &slotIndex, 0);
}

const uint8_t *rawDataPoolSlotPtr(uint16_t slotIndex)
{
    if (slotIndex >= RAWDATA_POOL_SLOT_COUNT)
    {
        return nullptr;
    }
    return rawDataPool[slotIndex];
}

uint16_t rawDataPoolFreeCount()
{
    if (rawDataFreeSlots == NULL)
    {
        return 0;
    }
    return static_cast<uint16_t>(uxQueueMessagesWaiting(rawDataFreeSlots));
}

// Define the rawdata publish task
void publishRawTask(void *pvParameters)
{
    RawPublishMessage queue_out;

    while (true)
    {
        while (mqtt_client.state() != MQTT_CONNECTED || !isWifiConnected)
        {
            vTaskDelay(pdMS_TO_TICKS(100)); // Wait until MQTT is connected
        }

        if (xQueueReceive(rawPublishQueue, &queue_out, 0) == pdTRUE)
        {
            const uint8_t *payloadPtr = rawDataPoolSlotPtr(queue_out.slot_index);
            if (payloadPtr != nullptr)
            {
                bool success = mqtt_client.publish(queue_out.topic, payloadPtr, queue_out.payload_len);
                if (!success)
                {
                    String failMsg = "MQTT rawdata publish failed: " + String(queue_out.topic);
                    DEBUG_PRINTLN(failMsg);
                }
            }

            rawDataPoolFreeSlot(queue_out.slot_index);
        }

        vTaskDelay(pdMS_TO_TICKS(publishInterval));
    }
}

bool ensureRawPublishInfraInitialized()
{
    std::lock_guard<std::mutex> lock(rawInitMutex);
    if (rawInfraInitialized)
    {
        return true;
    }

    rawDataFreeSlots = xQueueCreate(RAWDATA_POOL_SLOT_COUNT, sizeof(uint16_t));
    rawPublishQueue = xQueueCreate(RAWDATA_POOL_SLOT_COUNT, sizeof(RawPublishMessage));
    if (rawDataFreeSlots == NULL || rawPublishQueue == NULL)
    {
        DEBUG_PRINTLN("Failed to create rawdata queue or pool");
        return false;
    }

    for (uint16_t idx = 0; idx < RAWDATA_POOL_SLOT_COUNT; ++idx)
    {
        if (xQueueSend(rawDataFreeSlots, &idx, 0) != pdTRUE)
        {
            DEBUG_PRINTLN("Failed to initialize rawdata pool free slots");
            return false;
        }
    }

    if (xTaskCreate(publishRawTask, "Publish Raw Task", 4096, NULL, 1, NULL) != pdPASS)
    {
        DEBUG_PRINTLN("Failed to create rawdata publish task");
        return false;
    }

    rawInfraInitialized = true;
    DEBUG_PRINTLN("Rawdata publish infra initialized");
    return true;
}

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
            uint8_t currentQueueSize = uxQueueMessagesWaiting(publishQueue);
            maxUsedQueueSize = max(maxUsedQueueSize, currentQueueSize); // to track max used queue size for debugging purposes, this musst be under 150
            if (maxUsedQueueSize > oldMaxUsedQueueSize)
            {
                oldMaxUsedQueueSize = maxUsedQueueSize;
                setState("maxpubqueue", String(maxUsedQueueSize), true);
            }
            
            //  Call the publish function
            bool success = mqtt_client.publish(queue_out.topic, queue_out.payload);
            if (!success)
            {
                String failMsg = "MQTT publish failed: " + String(queue_out.topic);
                DEBUG_PRINTLN(failMsg);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(publishInterval)); // time between publish attempts, can be adjust via MQTT, default is 50ms, which means max 20 publishes per second, adjust if you have a lot of messages to publish and the queue is filling up, but be careful with too low values as it can cause stability issues with the MQTT client  
    }
}

void publish_init()
{
    // Create the publishqueue
    publishQueue = xQueueCreate(150u, sizeof(PublishMessage)); // Queue can hold 150 messages, adjust as needed
    if (publishQueue == NULL)
    {
        DEBUG_PRINTLN("Failed to create publish queue"); //without this, the system cannot function properly, so we restart to try again
        ESP.restart(); // Restart if queue creation fails
    }
    else {
        DEBUG_PRINTLN("Publish queue created successfully");
    }

    // Create the publish task
    xTaskCreate(publishTask, "Publish Task", 4096, NULL, 1, NULL);
    DEBUG_PRINTLN("Publish task created");
}