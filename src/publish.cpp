#include "publish.h"
#include <esp_heap_caps.h>

QueueHandle_t publishQueue = NULL;
QueueHandle_t rawPublishQueue = NULL;
UBaseType_t publishQueueCount = PUBLISH_QUEUE_COUNT;

UBaseType_t maxUsedQueueSize = 0;
UBaseType_t oldMaxUsedQueueSize = 0;

static StaticQueue_t publishQueueControlBlock;
static uint8_t *publishQueueStoragePsram = nullptr;

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
        while (true)
        {
            bool mqttConnected = false;
            {
                std::lock_guard<std::mutex> ioLock(mqttClientIoMutex);
                mqttConnected = (mqtt_client.state() == MQTT_CONNECTED);
            }
            if (mqttConnected && isWifiConnected)
            {
                break;
            }
            vTaskDelay(pdMS_TO_TICKS(100)); // Wait until MQTT is connected
        }

        if (xQueueReceive(rawPublishQueue, &queue_out, portMAX_DELAY) == pdTRUE)
        {
            const uint8_t *payloadPtr = rawDataPoolSlotPtr(queue_out.slot_index);
            if (payloadPtr != nullptr)
            {
                bool success = false;
                {
                    std::lock_guard<std::mutex> ioLock(mqttClientIoMutex);
                    success = mqtt_client.publish(queue_out.topic, payloadPtr, queue_out.payload_len);
                }
                if (!success)
                {
                    String failMsg = "MQTT rawdata publish failed: " + String(queue_out.topic);
                    DEBUG_PRINTLN(failMsg);
                    {
                        std::lock_guard<std::mutex> ioLock(mqttClientIoMutex);
                        mqtt_client.disconnect();
                    }
                }
                else
                {
                    incrementPublishedMessageCounter();
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
        bool mqttConnected = false;
        {
            std::lock_guard<std::mutex> ioLock(mqttClientIoMutex);
            mqttConnected = (mqtt_client.state() == MQTT_CONNECTED);
        }

        // publish messages from the queue as long as MQTT is connected, WiFi is available and there are messages in the queue
        // if not, wait until connection is back before trying to publish again
        while (mqttConnected && isWifiConnected && xQueueReceive(publishQueue, &queue_out, portMAX_DELAY) == pdTRUE)
        {
            UBaseType_t currentQueueSize = uxQueueMessagesWaiting(publishQueue);
            maxUsedQueueSize = max(maxUsedQueueSize, currentQueueSize); // to track max used queue size for debugging purposes,
            // this musst be under publishQueueCount, if you see this value is close to publishQueueCount,
            // you should consider increasing the queue size or publish interval to avoid dropping messages
            
            // for debugging purposes, print the current queue size and max used queue size
            if (debug_flg)
            {
                DEBUG_PRINTLN("Current publish queue size: " + String(currentQueueSize) + ", Max used queue size (till now): " + String(maxUsedQueueSize));
            }
                        
            // setState only if the maxUsedQueueSize has changed to avoid unnecessary MQTT publishes
            if (maxUsedQueueSize > oldMaxUsedQueueSize)
            {
                oldMaxUsedQueueSize = maxUsedQueueSize;
                setState("maxpubqueue", String(maxUsedQueueSize), true);
            }
            
            //  Call the publish function
            bool success = false;
            {
                std::lock_guard<std::mutex> ioLock(mqttClientIoMutex);
                success = mqtt_client.publish(queue_out.topic, queue_out.payload, queue_out.retain);
            }
            if (!success)
            {
                String failMsg = "MQTT publish failed: " + String(queue_out.topic);
                DEBUG_PRINTLN(failMsg);
                {
                    std::lock_guard<std::mutex> ioLock(mqttClientIoMutex);
                    mqtt_client.disconnect();
                }
                break;
            }

            incrementPublishedMessageCounter();

            {
                std::lock_guard<std::mutex> ioLock(mqttClientIoMutex);
                mqttConnected = (mqtt_client.state() == MQTT_CONNECTED);
            }
            vTaskDelay(pdMS_TO_TICKS(publishInterval)); // time between publish attempts, can be adjust via MQTT, default is 50ms,
            // which means max 20 publishes per second, adjust if you have a lot of messages to publish and the queue is filling up,
            // but be careful with too low values as it can cause stability issues with the MQTT client
        } 
    }
}

void publish_init()
{
    publishQueueCount = PUBLISH_QUEUE_COUNT; // fallback for boards without PSRAM

#ifdef BOARD_HAS_PSRAM
    if (psramFound())
    {
        size_t psramTotal = ESP.getPsramSize();
        size_t targetBytes = psramTotal / 2; // budget half of the installed PSRAM for the publish queue
        UBaseType_t psramCount = static_cast<UBaseType_t>(targetBytes / sizeof(PublishMessage));
        if (psramCount > publishQueueCount)
        {
            publishQueueCount = psramCount;
        }

        size_t storageBytes = static_cast<size_t>(publishQueueCount) * sizeof(PublishMessage);
        // MALLOC_CAP_SPIRAM only succeeds if the memory is really taken from PSRAM - this is our proof of placement
        publishQueueStoragePsram = static_cast<uint8_t *>(heap_caps_malloc(storageBytes, MALLOC_CAP_SPIRAM));
        if (publishQueueStoragePsram != nullptr)
        {
            publishQueue = xQueueCreateStatic(publishQueueCount, sizeof(PublishMessage), publishQueueStoragePsram, &publishQueueControlBlock);
            DEBUG_PRINTLN("Publish-Queue mit " + String(publishQueueCount) + " Eintraegen erfolgreich im PSRAM angelegt (" + String(storageBytes) + " Bytes)");
        }
        else
        {
            DEBUG_PRINTLN("PSRAM-Allokation fuer Publish-Queue fehlgeschlagen, falle auf Standardgroesse im internen RAM zurueck");
            publishQueueCount = PUBLISH_QUEUE_COUNT;
        }
    }
#endif

    if (publishQueue == NULL)
    {
        // Create the publishqueue in internal RAM (default heap) - fallback path or non-PSRAM boards
        publishQueue = xQueueCreate(publishQueueCount, sizeof(PublishMessage));
    }

    if (publishQueue == NULL)
    {
        DEBUG_PRINTLN("Failed to create publish queue"); //without this, the system cannot function properly, so we restart to try again
        ESP.restart(); // Restart if queue creation fails
    }
    else {
        DEBUG_PRINTLN("Publish queue created successfully (depth: " + String(publishQueueCount) + ")");
    }

    // Create the publish task
    xTaskCreate(publishTask, "Publish Task", 4096, NULL, 1, NULL);
    DEBUG_PRINTLN("Publish task created");
}