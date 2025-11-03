#include "TelegramHelper.h"
#include <Arduino.h>

// =================================================================
// Constructor Completo
// =================================================================
TelegramHelper::TelegramHelper(const char* botToken, LedControl& led23, Adafruit_SSD1306& oled, const char* tsKey, 
                               float (*potFunc)(), bool (*dhtFunc)(float&, float&))
    : ledGreen(led23), display(oled), thingSpeakApiKey(tsKey), 
      readPotentiometer(potFunc), readDHT22(dhtFunc) {
    
    // Inicializa el puntero del bot con el token y el cliente seguro
    bot = new UniversalTelegramBot(botToken, secured_client);

    // Telegram usa HTTPS, se necesita confiar en el certificado de Telegram
    secured_client.setInsecure(); // Simplifica la conexión para pruebas
    Serial.println("TelegramHelper inicializado con dependencias.");
}

// =================================================================
// Conexión WiFi
// =================================================================
void TelegramHelper::inicializarConexion(const char* ssid, const char* password) {
    Serial.println("Iniciando conexión WiFi...");
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    Serial.print("WiFi conectado. IP: ");
    Serial.println(WiFi.localIP());
}

// =================================================================
// Lógica de Envío de Mensajes y Botones (Menú)
// =================================================================

// Envía el menú (para /start)
void TelegramHelper::sendMenu(String chat_id) {
    String menu = "Hola! Soy el ESP32. Estos son mis comandos:\n";
    menu += "1. /start - Mostrar este menú.\n";
    menu += "2. /led<pin><on/off> - Controlar LED (ej: /led23on).\n";
    menu += "3. /dht22 - Obtener Temp/Humedad.\n";
    menu += "4. /pote - Obtener voltaje del potenciómetro.\n";
    menu += "5. /platiot - Enviar DHT22 a ThingSpeak.\n";
    menu += "6. /display<comp> - Mostrar estado (ej: /displayled).\n";
    
    // Botones para facilidad de uso
    String keyboard = "[[\"/led23on\", \"/led23off\"], [\"/dht22\", \"/pote\"], [\"/platiot\"], [\"/displayled\"]]";
    bot->sendMessageWithReplyKeyboard(chat_id, menu, "", keyboard);
}

// Envía un mensaje. Usa botones si se solicita.
void TelegramHelper::sendMessage(String chat_id, String text, bool useButtons) {
    if (useButtons) {
        // Usa el teclado de botones definido en sendMenu
        String keyboard = "[[\"/led23on\", \"/led23off\"], [\"/dht22\", \"/pote\"], [\"/platiot\"], [\"/displayled\"]]";
        bot->sendMessageWithReplyKeyboard(chat_id, text, "", keyboard);
    } else {
        bot->sendMessage(chat_id, text, "");
    }
}

// =================================================================
// Lógica de Comando LED
// =================================================================
void TelegramHelper::processLedCommand(String chat_id, String text) {
    // text será /led23on o /led23off (ya en minúsculas)
    int pin = 23; 
    String estado;
    bool success = false;

    if (text.endsWith("on")) {
        ledGreen.toggle(true);
        estado = "Encendido";
        success = true;
    } else if (text.endsWith("off")) {
        ledGreen.toggle(false);
        estado = "Apagado";
        success = true;
    } 
    
    if (success) {
            sendMessage(chat_id, String(ledGreen.getStatus() ? "🟢" : "⚫") + " LED (GPIO " + String(pin) + ") " + estado + ".");        // Actualizamos la pantalla
        mostrarEnDisplay(display, "LED (GPIO 23):\n" + estado);
    } else {
        sendMessage(chat_id, "Comando de LED no válido. Usa /led23on o /led23off.");
    }
}


// =================================================================
// Procesamiento de Comandos (handleNewMessages)
// =================================================================
void TelegramHelper::handleNewMessages(int numNewMessages) {
    for (int i = 0; i < numNewMessages; i++) {
        String chat_id = bot->messages[i].chat_id;
        String text = bot->messages[i].text;
        text.toLowerCase(); // Convertir a minúsculas para facilitar la comparación

        // 1. /start
        if (text == "/start") {
            sendMenu(chat_id);
        }
        
        // 2. /led<pin><on/off>
        else if (text.startsWith("/led23")) {
            processLedCommand(chat_id, text);
        }

        // 3. /dht22
        else if (text == "/dht22") {
            float temp, hum;
            if (readDHT22(temp, hum)) {
                String msg = "🌡️ Temp: " + String(temp, 1) + " °C\n";
                msg += "💧 Hum: " + String(hum, 1) + " %";
                sendMessage(chat_id, msg);
            } else {
                sendMessage(chat_id, "❌ Error al leer sensor DHT22. Comprueba la conexión del sensor.");
            }
        }

        // 4. /pote
        else if (text == "/pote") {
            float voltage = readPotentiometer();
            String msg = "🔌 Potenciómetro: " + String(voltage, 2) + " V";
            sendMessage(chat_id, msg);
        }
        
        // 5. /platiot
        else if (text == "/platiot") {
            float temp, hum;
            if (readDHT22(temp, hum)) {
                sendMessage(chat_id, "⏳ Enviando datos a ThingSpeak...");
                if (enviarThingSpeak(thingSpeakApiKey, temp, hum)) { 
                    sendMessage(chat_id, "✅ Datos (" + String(temp, 1) + "°C, " + String(hum, 1) + "%) enviados a ThingSpeak.");
                } else {
                    sendMessage(chat_id, "❌ Fallo al enviar datos a ThingSpeak. Revisa la clave.");
                }
            } else {
                sendMessage(chat_id, "❌ No se puede enviar: Error al leer sensor DHT22.");
            }
        }
        
        // 6. /display
        else if (text.startsWith("/display")) {
            String component = text.substring(8); 
            String msg = "Actualizando Pantalla OLED...";
            
            if (component == "led") {
                String estado = ledGreen.getStatus() ? "Encendido" : "Apagado";
                msg += "\nEstado LED: " + estado;
                mostrarEnDisplay(display, "LED (GPIO 23):\n" + estado);
            } else if (component == "pote") {
                float voltage = readPotentiometer();
                msg += "\nPotenciómetro: " + String(voltage, 2) + "V";
                mostrarEnDisplay(display, "POT:\n" + String(voltage, 2) + "V");
            } else if (component == "dht22") {
                float temp, hum;
                if (readDHT22(temp, hum)) {
                    msg += "\nTemp: " + String(temp, 1) + "C, Hum: " + String(hum, 1) + "%";
                    mostrarEnDisplay(display, "T: " + String(temp, 1) + "C\nH: " + String(hum, 1) + "%");
                } else {
                    msg += "\nDHT22: Error de lectura.";
                    mostrarEnDisplay(display, "DHT22\nError");
                }
            } else {
                msg = "Comando de Display no reconocido. Usa /displayled, /displaypote o /displaydht22.";
                mostrarEnDisplay(display, "Display\nCMD No ID");
            }
            sendMessage(chat_id, msg);
        }

        // Comando no reconocido
        else {
            sendMessage(chat_id, "Comando no reconocido. Usa /start para ver el menú.", true);
        }
    }
}

// =================================================================
// Chequeo de Mensajes (loop)
// =================================================================
void TelegramHelper::verificarMensajes() {
    if (WiFi.status() != WL_CONNECTED) {
        // Opcional: intentar reconectar o simplemente esperar
        return;
    }
    
    if (millis() - lastCheck > checkInterval) {
        int numNewMessages = bot->getUpdates(bot->last_message_received + 1);
        while (numNewMessages) {
            // Serial.println("Mensaje recibido!"); // Ya se imprime dentro de handleNewMessages
            handleNewMessages(numNewMessages);
            numNewMessages = bot->getUpdates(bot->last_message_received + 1);
        }
        lastCheck = millis();
    }
}