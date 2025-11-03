#include "ThingSpeakHelper.h"

// La función ahora devuelve 'true' si el envío fue exitoso.
bool enviarThingSpeak(const char* apiKey, float temp, float hum) {
    HTTPClient http;
    
    // 1. Construcción de la URL
    String url = "http://api.thingspeak.com/update"; // Usamos HTTP simple, ya que ThingSpeak lo soporta
    url += "?api_key=";
    url += apiKey; // Usamos el parámetro
    url += "&field1=" + String(temp);
    url += "&field2=" + String(hum);

    // 2. Comienzo de la petición
    http.begin(url);
    int httpResponseCode = http.GET();
    
    bool exito = false;

    // 3. Verificación de la respuesta
    if (httpResponseCode > 0) {
        Serial.printf("[ThingSpeak] HTTP Response code: %d\n", httpResponseCode);
        // ThingSpeak devuelve 200 (OK) si la actualización fue exitosa
        if (httpResponseCode == 200) { 
            exito = true;
        }
    } else {
        Serial.printf("[ThingSpeak] Error en HTTP GET: %s\n", http.errorToString(httpResponseCode).c_str());
    }

    http.end();
    return exito;
}