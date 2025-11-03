#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h> // Para la pantalla OLED
#include <Adafruit_SSD1306.h> // Necesario para definir la instancia de display
#include <DHT.h> // Librería del sensor DHT (ej: Adafruit)

// 1. INCLUSIÓN DE LIBRERÍAS PROPIAS Y SECRETOS
#include "secrets.h" 
#include "DisplayHelper.h"    // inicializarDisplay, mostrarEnDisplay
#include "ThingSpeakHelper.h" // enviarThingSpeak (aunque llamado por TelegramHelper)
#include "TelegramHelper.h"    // Clase TelegramHelper

// 2. DEFINICIÓN DE CONSTANTES
#define LED_PIN         23  // Pin del LED que el bot controlará
#define DHT_PIN         15  // Pin donde está conectado el DHT22
#define POT_PIN         34  // Pin ADC para el Potenciómetro (ej: GPIO34)

#define SCREEN_WIDTH    128 // Ancho de la pantalla OLED
#define SCREEN_HEIGHT   64  // Alto de la pantalla OLED
#define OLED_RESET      -1  // Pin de Reset (usa -1 si está compartido con VCC)
#define SCREEN_ADDRESS  0x3C // Dirección I2C típica para OLED 128x64

void handleNewMessages(int numNewMessages);


// =============================================================
// 3. DECLARACIÓN DE OBJETOS DE HARDWARE (LVALUES)
// =============================================================
// Crear el objeto OLED (¡IMPORTANTE: Debe ser global/static para el TelegramHelper!)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Crear el objeto LED (usa la clase auxiliar de TelegramHelper.h)
LedControl led23(LED_PIN);

// Crear el objeto DHT22
DHT dht(DHT_PIN, DHT22); // Reemplaza DHT22 por DHT11 si usas ese modelo

// Declaración forward del bot (Se inicializará en setup)
TelegramHelper *myBot;

// =============================================================
// 4. FUNCIONES DE LECTURA DE HARDWARE (CALLBACKS)
// =============================================================

/**
 * @brief Lee el valor del potenciómetro y lo convierte a voltaje (0.0 a 3.3V).
 * @return Voltaje flotante.
 */
float readPot() {
    // El ESP32 lee de 0 a 4095 por defecto (12 bits ADC)
    float analogValue = analogRead(POT_PIN);
    // Mapeo a voltaje (asumiendo alimentación de 3.3V)
    return (analogValue * 3.3) / 4095.0; 
}

/**
 * @brief Lee los valores de temperatura y humedad del DHT22.
 * @param temp Referencia para almacenar la temperatura.
 * @param hum Referencia para almacenar la humedad.
 * @return true si la lectura es exitosa, false si hay error.
 */
bool readDht(float &temp, float &hum) {
    // Lectura del sensor
    hum = dht.readHumidity();
    temp = dht.readTemperature();

    // Comprobar si la lectura es válida (DHT devuelve NaN si falla)
    if (isnan(hum) || isnan(temp)) {
        Serial.println(F("Error de lectura del sensor DHT."));
        return false;
    }
    return true;
}

void setup() {
    Serial.begin(115200);
    dht.begin(); // Inicializa el sensor DHT

    // 1. Inicialización de la Pantalla
    if (!inicializarDisplay(display)) { // Llama a la función de DisplayHelper.cpp
        Serial.println(F("Fallo al inicializar la pantalla OLED."));
        // No detener la ejecución, continuar sin display
    }
    mostrarEnDisplay(display, "Inicializando...");

    // 2. Inicialización del Bot
    // Creamos el objeto del bot pasándole el token y TODAS las dependencias (Lvalues)
    myBot = new TelegramHelper(
        BOT_TOKEN, 
        led23, 
        display, 
        THINGSPEAK_API_KEY, 
        readPot, 
        readDht
    );

    // 3. Conexión a WiFi (delegada al Helper)
    myBot->inicializarConexion(WIFI_SSID, WIFI_PASSWORD);
    
    // 4. Mostrar estado de conexión en pantalla
    if (WiFi.status() == WL_CONNECTED) {
        mostrarEnDisplay(display, "Conectado!\nIP:\n" + WiFi.localIP().toString());
    } else {
        mostrarEnDisplay(display, "WiFi\nERROR!");
    }
}
void loop() {

  myBot->verificarMensajes();
}
