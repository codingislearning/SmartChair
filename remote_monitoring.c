// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "iot_configs.h"

#include "AzureIoTHub.h"
#include "sdk/schemaserializer.h"
#include "try.h"
/* CODEFIRST_OK is the new name for IOT_AGENT_OK. The follow #ifndef helps during the name migration. Remove it when the migration ends. */
#ifndef  IOT_AGENT_OK
#define  IOT_AGENT_OK CODEFIRST_OK
#endif // ! IOT_AGENT_OK

#define MAX_DEVICE_ID_SIZE  20



int basePressure;
int leftLowerBackPressure;
int rightLowerBackPressure;
int upperBackPressure;

// Define the Model
BEGIN_NAMESPACE(SmartChair);

DECLARE_MODEL(PressureSensorGrid,
    WITH_DATA(int, Base),
    WITH_DATA(int, LeftLowerBack),
    WITH_DATA(int, UpperBack),
    WITH_DATA(int, RightLowerBack),
    WITH_DATA(bool, Alarm),
    WITH_DATA(ascii_char_ptr, DeviceId),
    WITH_ACTION(SetAlarmCommand,bool,turnOn)
);

END_NAMESPACE(SmartChair);
EXECUTE_COMMAND_RESULT SetAlarmCommand(PressureSensorGrid* grid, bool turnOn)
{
    LogInfo("Received command %d\r\n", turnOn);
    grid->Alarm = turnOn;
    return EXECUTE_COMMAND_SUCCESS;
}
//
//EXECUTE_COMMAND_RESULT SetHumidity(Thermostat* thermostat, int humidity)
//{
//    LogInfo("Received humidity %d\r\n", humidity);
//    thermostat->Humidity = humidity;
//    return EXECUTE_COMMAND_SUCCESS;
//}

static void sendMessage(IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle, const unsigned char* buffer, size_t size)
{
    IOTHUB_MESSAGE_HANDLE messageHandle = IoTHubMessage_CreateFromByteArray(buffer, size);
    if (messageHandle == NULL)
    {
        LogInfo("unable to create a new IoTHubMessage\r\n");
    }
    else
    {
        if (IoTHubClient_LL_SendEventAsync(iotHubClientHandle, messageHandle, NULL, NULL) != IOTHUB_CLIENT_OK)
        {
            LogInfo("failed to hand over the message to IoTHubClient");
        }
        else
        {
            LogInfo("IoTHubClient accepted the message for delivery\r\n");
        }

        IoTHubMessage_Destroy(messageHandle);
    }
    free((void*)buffer);
}

static size_t GetDeviceId(const char* connectionString, char* deviceID, size_t size)
{
    size_t result;
    const char* runStr = connectionString;
    char ustate = 0;
    char* start = NULL;

    if (runStr == NULL)
    {
        result = 0;
    }
    else
    {
        while (*runStr != '\0')
        {
            if (ustate == 0)
            {
                if (strncmp(runStr, "DeviceId=", 9) == 0)
                {
                    runStr += 9;
                    start = runStr;
                }
                ustate = 1;
            }
            else
            {
                if (*runStr == ';')
                {
                    if (start == NULL)
                    {
                        ustate = 0;
                    }
                    else
                    {
                        break;
                    }
                }
                runStr++;
            }
        }

        if (start == NULL)
        {
            result = 0;
        }
        else
        {
            result = runStr - start;
            if (deviceID != NULL)
            {
                for (size_t i = 0; ((i < size - 1) && (start < runStr)); i++)
                {
                    *deviceID++ = *start++;
                }
                *deviceID = '\0';
            }
        }
    }

    return result;
}

/*this function "links" IoTHub to the serialization library*/
static IOTHUBMESSAGE_DISPOSITION_RESULT IoTHubMessage(IOTHUB_MESSAGE_HANDLE message, void* userContextCallback)
{
    IOTHUBMESSAGE_DISPOSITION_RESULT result;
    const unsigned char* buffer;
    size_t size;
    if (IoTHubMessage_GetByteArray(message, &buffer, &size) != IOTHUB_MESSAGE_OK)
    {
        LogInfo("unable to IoTHubMessage_GetByteArray\r\n");
        result = EXECUTE_COMMAND_ERROR;
    }
    else
    {
        /*buffer is not zero terminated*/
        char* temp = malloc(size + 1);
        if (temp == NULL)
        {
            LogInfo("failed to malloc\r\n");
            result = EXECUTE_COMMAND_ERROR;
        }
        else
        {
            EXECUTE_COMMAND_RESULT executeCommandResult;

            memcpy(temp, buffer, size);
            temp[size] = '\0';
            
            LogInfo("Executing Command");
            executeCommandResult = EXECUTE_COMMAND(userContextCallback, temp);
            result =
                (executeCommandResult == EXECUTE_COMMAND_ERROR) ? IOTHUBMESSAGE_ABANDONED :
                (executeCommandResult == EXECUTE_COMMAND_SUCCESS) ? IOTHUBMESSAGE_ACCEPTED :
                IOTHUBMESSAGE_REJECTED;
            free(temp);
        }
    }
    return result;
}

void remote_monitoring_run()
{
 
   
        srand((unsigned int)time(NULL));
        InitSoftwareSerial();
        InitAlarm();
        if (serializer_init(NULL) != SERIALIZER_OK)
        {
            LogInfo("Failed on serializer_init\r\n");
        }
        else
        {
            IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle;

#if defined(IOT_CONFIG_MQTT)
            iotHubClientHandle = IoTHubClient_LL_CreateFromConnectionString(IOT_CONFIG_CONNECTION_STRING, MQTT_Protocol);
#elif defined(IOT_CONFIG_HTTP)
            iotHubClientHandle = IoTHubClient_LL_CreateFromConnectionString(IOT_CONFIG_CONNECTION_STRING, HTTP_Protocol);
#else
            iotHubClientHandle = NULL;
#endif

            if (iotHubClientHandle == NULL)
            {
                LogInfo("Failed on IoTHubClient_CreateFromConnectionString\r\n");
            }
            else
            {

                PressureSensorGrid* grid = CREATE_MODEL_INSTANCE(SmartChair, PressureSensorGrid);
                if (grid == NULL)
                {
                    LogInfo("Failed on CREATE_MODEL_INSTANCE\r\n");
                }
                else
                {
                    if (IoTHubClient_LL_SetMessageCallback(iotHubClientHandle, IoTHubMessage, grid) != IOTHUB_CLIENT_OK)
                    {
                        LogInfo("ERROR: IoTHubClient_LL_SetMessageCallback..........FAILED!\r\n");
                    }
                    else
                    {
                        LogInfo("IoTHubClient_LL_SetMessageCallback...successful.\r\n");
                    }

                    {

                        char deviceId[MAX_DEVICE_ID_SIZE];
                        if (GetDeviceId(IOT_CONFIG_CONNECTION_STRING, deviceId, MAX_DEVICE_ID_SIZE) > 0)
                        {
                            LogInfo("deviceId=%s", deviceId);
                        }

                        /* send the device info upon startup so that the cloud app knows
                        what commands are available and the fact that the device is up */

                        grid->DeviceId = (char*)deviceId;
                        int sendCycle = 3;
                        int currentCycle = 0;
                        unsigned char*buffer;
                        size_t bufferSize;
                        int sensorData[4];
                        while (1)
                        {
                            if(currentCycle >= sendCycle) {
                                  getNextSample(sensorData);
                                  basePressure = sensorData[0];
                                  leftLowerBackPressure = sensorData[1];
                                  rightLowerBackPressure = sensorData[2];
                                  upperBackPressure = sensorData[3];
                          
                                  
                                  grid->Base = basePressure;
                                  grid->LeftLowerBack = leftLowerBackPressure;
                                  grid->RightLowerBack = rightLowerBackPressure;
                                  grid->UpperBack = upperBackPressure;
                          
                                  currentCycle = 0;
                                  if(grid->Alarm)
                                  {
                                    SetAlarm();
                                  }
                                  else
                                  {
                                    UnSetAlarm();
                                  }
                          
                                  LogInfo("Sending sensor value Base = %d, LeftLowerBack = %d, RightLowerBack = %d, UpperBack = %d\r\n", grid->Base, grid->LeftLowerBack, grid->RightLowerBack, grid->UpperBack);
                          
                                  if (SERIALIZE(&buffer, &bufferSize, grid->DeviceId, grid->Base, grid->LeftLowerBack, grid->RightLowerBack, grid->UpperBack) != IOT_AGENT_OK)
                                  {
                                    LogInfo("Failed sending sensor value\r\n");
                                  }
                                  else
                                  {
                                    sendMessage(iotHubClientHandle, buffer, bufferSize);
                                  }
                            }

                            IoTHubClient_LL_DoWork(iotHubClientHandle);
                            ThreadAPI_Sleep(1000);
                            currentCycle++;
                        }
                    }

                    DESTROY_MODEL_INSTANCE(grid);
                }
                IoTHubClient_LL_Destroy(iotHubClientHandle);
            }
            serializer_deinit();

    }
}
