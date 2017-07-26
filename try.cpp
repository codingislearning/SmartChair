// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <Arduino.h>
#include "ESPSoftwareSerial.h"
#include "try.h"

#define SoftwareSerialRx D6      
#define SoftwareSerialTx D7

#define AlarmPin D1

// See guide for details on sensor wiring and usage:
//   https://learn.adafruit.com/dht/overview
SoftwareSerial myserial(SoftwareSerialRx, SoftwareSerialTx);
uint8_t sensor;

void InitSoftwareSerial(void)
{
	myserial.begin(4800);
	delay(100);
}

void InitAlarm()
{
  pinMode(AlarmPin, OUTPUT);
}

uint8_t readByte()
{
	byte data;
	if (myserial.available())
	{
		data = myserial.read();
		return data;
	}
	else
	{
		yield();
		myserial.write(sensor);
    delay(10);
		Serial.println("Recursivecall");
    Serial.print(sensor);
		delay(500);
		readByte();
	}
}

int receiveInt16()
{
	int count = 0;
	int data = 0;
	while (count < 2)
	{
		data = data * 256 + readByte();
		count++;
	}
	return data;
}



void getNextSample(int* sensorData)
{
	int data = -1;

	for (sensor = 0; sensor < 4; sensor++)
	{
		myserial.write(sensor);
		delay(10);
		Serial.print("request:");
		Serial.println(sensor);
    delay(100);
		data = receiveInt16();
		Serial.print("received");
		Serial.println(data);
		sensorData[sensor] = data;
	}
	
}

void SetAlarm()
{
  digitalWrite(AlarmPin, HIGH);
}
void UnSetAlarm()
{
  digitalWrite(AlarmPin, LOW);
}

