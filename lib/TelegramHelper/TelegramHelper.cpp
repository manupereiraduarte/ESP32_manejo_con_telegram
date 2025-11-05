#include "TelegramHelper.h"
#include <Arduino.h>

// =================================================================
// Constructor Completo
// =================================================================
TelegramHelper::TelegramHelper(const char* botToken, LedControl& led23, U8G2_SSD1306_128X64_NONAME_F_SW_I2C& oled, const char* tsKey, 
                               float (*potFunc)(), bool (*dhtFunc)(float&, float&))
    : ledGreen(led23), display(oled), thingSpeakApiKey(tsKey), 
      readPotentiometer(potFunc), readDHT22(dhtFunc) {
    
    bot = new UniversalTelegramBot(botToken, secured_client);
    secured_client.setInsecure();
    Serial.println("TelegramHelper inicializado con dependencias.");
}

// =================================================================
// Conexión WiFi
// =================================================================
void TelegramHelper::inicializarConexion(const char* ssid, const char* password) {
    Serial.println("Iniciando conexión WiFi...");
    WiFi.begin(ssid, password);

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 20000) {
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    if (WiFi.status() == WL_CONNECTED) {
        Serial.print("WiFi conectado. IP: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("❌ Error: No se pudo conectar al WiFi.");
    }
}

// =================================================================
// Lógica de Envío de Mensajes y Botones (Menú)
// =================================================================

// Envía el menú (para /start)
void TelegramHelper::sendMenu(String chat_id) {
    String menu = "Hola! Soy el ESP32. Estos son mis comandos:\n";
    menu += "1. Encender/Apagar LED (GPIO 23).\n";
    menu += "2. Obtener valores de sensores.\n";
    menu += "3. Enviar datos a ThingSpeak.\n";
    menu += "4. Mostrar estados en Display OLED.\n";
    
    // CONSTRUCCIÓN DEL TECLADO DESCRIPTIVO (JSON)
    String keyboard = 
        "[[\"Encender LED\", \"Apagar LED\"],"
        "[\"Datos Sensor\", \"Voltaje Pote\"],"
        "[\"Enviar a IoT\", \"LED en Oled\"],"
        "[\"Sensor en Oled\", \"Voltaje en Oled\"]]";

    sendMessage(chat_id, menu, keyboard); 
}

// Envía un mensaje. Usa botones si se solicita.
void TelegramHelper::sendMessage(String chat_id, String text, String keyboard) {
    if (keyboard.length() > 0) {
        bot->sendMessageWithReplyKeyboard(chat_id, text, "", keyboard);
    } else {
        bot->sendMessage(chat_id, text, "");
    }
}

// =================================================================
// Lógica de Comando LED
// =================================================================
void TelegramHelper::processLedCommand(String chat_id, String text) {
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
        sendMessage(chat_id, String(ledGreen.getStatus() ? "🟢" : "⚫") + " LED (GPIO " + String(pin) + ") " + estado + ".");
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
        text.toLowerCase(); 

        // 1. FASE DE MAPEO: Convertir el texto descriptivo del botón al comando /...
        if (text == "encender led") {
            text = "/led23on";
        } else if (text == "apagar led") {
            text = "/led23off";
        } else if (text == "datos sensor") {
            text = "/dht22";
        } else if (text == "voltaje pote") {
            text = "/pote";
        } else if (text == "enviar a iot") {
            text = "/platiot";
        } else if (text == "led en oled") {
            text = "/displayled";
        } else if (text == "sensor en oled") {
            text = "/displaydht22";
        } else if (text == "voltaje en oled") {
            text = "/displaypote";
        }
        
        // 2. FASE DE EJECUCIÓN: Procesar el comando /... resultante
        
        // 1. /start
        if (text == "/start" || text == "/ayuda") {
            sendMenu(chat_id);
        }
        
        // 2. /led<pin><on/off>
        else if (text.startsWith("/led23")) {
            processLedCommand(chat_id, text);
        }
        
        // 3. /dht22 (Informar Temp/Hum)
        else if (text == "/dht22") {
            float temp, hum;
            if (readDHT22(temp, hum)) {
                String msg = "🌡️ Temp: " + String(temp, 1) + " °C\n";
                msg += "💧 Hum: " + String(hum, 1) + " %";
                sendMessage(chat_id, msg, ""); 
            } else {
                sendMessage(chat_id, "❌ Error al leer sensor DHT22.", "");
            }
        }
        
        // 4. /pote (Informar Voltaje)
        else if (text == "/pote") {
            float voltage = readPotentiometer();
            String msg = "🔌 Potenciómetro: " + String(voltage, 2) + " V";
            sendMessage(chat_id, msg, "");
        }
        
        // 5. /platiot (Enviar a ThingSpeak)
        else if (text == "/platiot") {
            float temp, hum;
            float pote = readPotentiometer();
            
            if (readDHT22(temp, hum)) {
                sendMessage(chat_id, "⏳ Enviando datos a ThingSpeak...", "");
                if (enviarThingSpeak(thingSpeakApiKey, temp, hum, pote)) { 
                    sendMessage(chat_id, "✅ Datos (T, H, Pote) enviados a ThingSpeak.", "");
                } else {
                    sendMessage(chat_id, "❌ Fallo al enviar datos a ThingSpeak.", "");
                }
            } else {
                sendMessage(chat_id, "❌ No se puede enviar: Error al leer sensor DHT22.", "");
            }
        }
        
        // 6. /display (Mostrar estado en OLED)
        else if (text.startsWith("/display")) {
            String component = text.substring(8);
            String msg = "Actualizando Pantalla OLED...";
            
            float temp, hum, voltage;

            if (component == "led") {
                String estado = ledGreen.getStatus() ? "Encendido" : "Apagado";
                msg += "\nEstado LED: " + estado;
                mostrarEnDisplay(display, "LED:\n" + estado);
            } else if (component == "pote") {
                voltage = readPotentiometer();
                msg += "\nPotenciómetro: " + String(voltage, 2) + "V";
                mostrarEnDisplay(display, "  POT:\n" + String(voltage, 2) + "V");
            } else if (component == "dht22") {
                if (readDHT22(temp, hum)) {
                    msg += "\nTemp: " + String(temp, 1) + "C, Hum: " + String(hum, 1) + "%";
                    mostrarEnDisplay(display, "   T: " + String(temp, 1) + "C\n   H: " + String(hum, 1) + "%");
                } else {
                    msg += "\nDHT22: Error de lectura.";
                    mostrarEnDisplay(display, " DHT22\nError");
                }
            } else {
                msg = "Comando de Display no reconocido.";
                mostrarEnDisplay(display, "Display\nCMD No ID");
            }
            sendMessage(chat_id, msg, "");
        }
        
        // Comandos no reconocidos
        else {
            sendMessage(chat_id, "Comando no reconocido. Usa /start para ver el menú.", "");
        }
    }
}

// =================================================================
// Chequeo de Mensajes (loop)
// =================================================================
void TelegramHelper::verificarMensajes() {
    if (WiFi.status() != WL_CONNECTED) {
        return;
    }
    
    if (millis() - lastCheck > checkInterval) {
        int numNewMessages = bot->getUpdates(bot->last_message_received + 1);
        while (numNewMessages) {
            handleNewMessages(numNewMessages);
            numNewMessages = bot->getUpdates(bot->last_message_received + 1);
        }
        lastCheck = millis();
    }
}
