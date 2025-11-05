#ifndef THINGSPEAKHELPER_H
#define THINGSPEAKHELPER_H

#include <HTTPClient.h>
#include <Arduino.h> 

bool enviarThingSpeak(const char* apiKey, float temp, float hum, float pote); 

#endif