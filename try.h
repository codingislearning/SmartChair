// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef __TRY_H
#define __TRY_H


#ifdef __cplusplus
extern "C" {
#endif
void InitAlarm(void);
void InitSoftwareSerial(void);
void getNextSample(int* sensorData);
void SetAlarm();
void UnSetAlarm();

#ifdef __cplusplus
}
#endif


#endif//__DHT22_H

