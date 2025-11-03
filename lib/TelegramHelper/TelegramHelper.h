#ifndef TELEGRAMHELPER_H
#define TELEGRAMHELPER_H

#include <UniversalTelegramBot.h>
#include <WiFiClientSecure.h>
#include <WiFi.h>
#include "DisplayHelper.h" // Incluimos para poder usar sus funciones
#include "ThingSpeakHelper.h" // Incluimos para poder usar sus funciones

// 🚨 Necesitas definir un LED: usaremos una clase auxiliar
class LedControl {
private:
    int ledPin;
    bool estado = false;
public:
    LedControl(int pin) : ledPin(pin) { pinMode(ledPin, OUTPUT); digitalWrite(ledPin, LOW); }
    bool toggle(bool on) { estado = on; digitalWrite(ledPin, estado ? HIGH : LOW); return estado; }
    bool getStatus() const { return estado; }
};

// =================================================================
// Clase Principal
// =================================================================
class TelegramHelper {
private:
    WiFiClientSecure secured_client;
    UniversalTelegramBot *bot;
    unsigned long lastCheck = 0;
    const unsigned long checkInterval = 2000;

    // Dependencias
    LedControl& ledGreen;       // LED Verde (GPIO 23)
    // LedControl& ledBlue;     // Si tuvieras más LEDs
    
    // Necesitamos pasar la instancia del display y la key de ThingSpeak
    Adafruit_SSD1306& display; 
    const char* thingSpeakApiKey;

    // Funciones del ESP32 (necesitarás crearlas y pasarlas)
    float (*readPotentiometer)();
    bool (*readDHT22)(float &temp, float &hum);

    // Funciones internas
    void handleNewMessages(int numNewMessages);
    void sendMenu(String chat_id);
    void processLedCommand(String chat_id, String text);

public:
    // Constructor con todas las dependencias
    TelegramHelper(const char* botToken, LedControl& led23, Adafruit_SSD1306& oled, const char* tsKey, 
                   float (*potFunc)(), bool (*dhtFunc)(float&, float&));

    void inicializarConexion(const char* ssid, const char* password);
    void verificarMensajes();
    void sendMessage(String chat_id, String text, bool useButtons = false);
};

#endif