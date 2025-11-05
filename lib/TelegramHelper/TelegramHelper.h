#ifndef TELEGRAMHELPER_H
#define TELEGRAMHELPER_H

#include <UniversalTelegramBot.h>
#include <WiFiClientSecure.h>
#include <WiFi.h>
#include <U8g2lib.h> 

#include "DisplayHelper.h" 
#include "ThingSpeakHelper.h"

class LedControl {
private:
    int ledPin;
    bool estado = false;
public:
    LedControl(int pin) : ledPin(pin) { pinMode(ledPin, OUTPUT); digitalWrite(ledPin, LOW); }
    bool toggle(bool on) { estado = on; digitalWrite(ledPin, estado ? HIGH : LOW); return estado; }
    bool getStatus() const { return estado; }
};

// Clase Principal
class TelegramHelper {
private:
    U8G2_SSD1306_128X64_NONAME_F_SW_I2C& display; 
    WiFiClientSecure secured_client;
    UniversalTelegramBot *bot;
    unsigned long lastCheck = 0;
    const unsigned long checkInterval = 2000;

    // Dependencias
    LedControl& ledGreen; 
    const char* thingSpeakApiKey;

    // Funciones del ESP32 
    float (*readPotentiometer)();
    bool (*readDHT22)(float &temp, float &hum);

    // Funciones internas
    void handleNewMessages(int numNewMessages);
    void sendMenu(String chat_id);
    void processLedCommand(String chat_id, String text);

public:
    // Constructor con todas las dependencias
    TelegramHelper(const char* botToken, 
        LedControl& led23,
        U8G2_SSD1306_128X64_NONAME_F_SW_I2C& oled,
        const char* tsKey, 
        float (*potFunc)(),
        bool (*dhtFunc)(float&, float&));

    void inicializarConexion(const char* ssid, const char* password);
    void verificarMensajes();
    void sendMessage(String chat_id, String text, String keyboard = "");
};

#endif