#ifndef THINGSPEAKHELPER_H
#define THINGSPEAKHELPER_H

#include <HTTPClient.h>
#include <Arduino.h> // Para String y Serial

// Se añade la API Key como parámetro y la función devuelve un booleano para el éxito
bool enviarThingSpeak(const char* apiKey, float temp, float hum, float pote); 

#endif