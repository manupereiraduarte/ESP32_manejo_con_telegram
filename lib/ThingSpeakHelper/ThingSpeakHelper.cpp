#include "ThingSpeakHelper.h"

// devuelve 'true' si el envío fue exitoso.
bool enviarThingSpeak(const char* apiKey, float temp, float hum, float pote) {
    HTTPClient http;
    
    // 1. Construcción de la URL
    String url = "http://api.thingspeak.com/update"; // Usamos HTTP simple por thingspeak
    url += "?api_key=";
    url += apiKey;
    url += "&field1=" + String(temp);
    url += "&field2=" + String(hum);
    url += "&field3=" + String(pote);

    //Comienzo de la petición
    http.begin(url);
    int httpResponseCode = http.GET();
    
    bool exito = false;

    // Verificación de la respuesta
    if (httpResponseCode > 0) {
        Serial.printf("[ThingSpeak] HTTP Response code: %d\n", httpResponseCode);

        if (httpResponseCode == 200) { 
            exito = true;
        }
    } else {
        Serial.printf("[ThingSpeak] Error en HTTP GET: %s\n", http.errorToString(httpResponseCode).c_str());
    }

    http.end();
    return exito;
}