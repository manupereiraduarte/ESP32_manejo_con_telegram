#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h> 
#include <Adafruit_SSD1306.h> 
#include <DHT.h> 
#include <U8g2lib.h>

// INCLUSIÓN DE LIBRERÍAS PROPIAS Y SECRETOS
#include "secrets.h" 
#include "DisplayHelper.h"   
#include "ThingSpeakHelper.h"
#include "TelegramHelper.h" 

// DEFINICIÓN DE CONSTANTES
#define LED_PIN         23 
#define DHT_PIN         33
#define POT_PIN         32 
#define PIN_SCL         22
#define PIN_SDA         21



void handleNewMessages(int numNewMessages);


// DECLARACIÓN DE OBJETOS DE HARDWARE
// Crear el objeto OLED 
U8G2_SSD1306_128X64_NONAME_F_SW_I2C display(U8G2_R0, PIN_SCL, PIN_SDA);
// Crear el objeto LED
LedControl led23(LED_PIN);

// Crear el objeto DHT22
DHT dht(DHT_PIN, DHT22); 

// Declaración forward del bot
TelegramHelper *myBot;

// FUNCIONES DE LECTURA DE HARDWARE


float readPot() {
    // El ESP32 lee de 0 a 4095 por defecto (12 bits ADC)
    float analogValue = analogRead(POT_PIN);
    // Mapeo a voltaje (asumiendo alimentación de 3.3V)
    return (analogValue * 3.3) / 4095.0; 
}

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
    dht.begin(); 

    // Inicialización de la Pantalla
    if (!inicializarDisplay(display)) { 
        Serial.println(F("Fallo al inicializar la pantalla OLED."));
        // No detener la ejecución, continuar sin display
    }
    mostrarEnDisplay(display, "Inicializando...");

    // Inicialización del Bot
    myBot = new TelegramHelper(
        BOT_TOKEN, 
        led23, 
        display, 
        THINGSPEAK_API_KEY, 
        readPot, 
        readDht
    );

    // Conexión a WiFi (delegada al Helper)
    myBot->inicializarConexion(WIFI_SSID, WIFI_PASSWORD);
    
    // Mostrar estado de conexión en pantalla
    if (WiFi.status() == WL_CONNECTED) {
        mostrarEnDisplay(display, "Conectado!\nIP:\n" + WiFi.localIP().toString());
    } else {
        mostrarEnDisplay(display, "WiFi\nERROR!");
    }
}
void loop() {

  myBot->verificarMensajes();
}
