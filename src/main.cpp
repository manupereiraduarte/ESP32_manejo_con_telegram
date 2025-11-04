#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h> 
#include <Adafruit_SSD1306.h>
#include <DHT.h> 
#include <U8g2lib.h>

// librerias propias y secrets para las claves
#include "secrets.h" 
#include "DisplayHelper.h"    
#include "ThingSpeakHelper.h" 
#include "TelegramHelper.h"    

// constantes de hardware
#define LED_PIN         23  
#define DHT_PIN         33 
#define POT_PIN         32  
#define PIN_SCL         22
#define PIN_SDA         21
#define LED_PIN_AZUL    2



void handleNewMessages(int numNewMessages);



// Declaración de objetos de hardware

// Creamos el oled
U8G2_SSD1306_128X64_NONAME_F_SW_I2C display(U8G2_R0, PIN_SCL, PIN_SDA); 

// creamos leds
LedControl led23(LED_PIN); 
LedControl led2(LED_PIN_AZUL);

// creamos sensor temp y humedad
DHT dht(DHT_PIN, DHT22); 

// Declaración forward del bot
TelegramHelper *myBot;


// lectura de potenciometro y DHT22

float readPot() {
    // El ESP32 lee de 0 a 4095 por defecto (12 bits ADC)
    float analogValue = analogRead(POT_PIN);
    // Mapeo a voltaje (asumiendo alimentación de 3.3V)
    return (analogValue * 3.3) / 4095.0; 
}

bool readDht(float &temp, float &hum) {
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

    if (!inicializarDisplay(display)) { // función de DisplayHelper
        Serial.println(F("Fallo al inicializar la pantalla OLED."));
    }
    mostrarEnDisplay(display, "Inicializando...");

    // Inicialización del Bot
    myBot = new TelegramHelper(
        BOT_TOKEN, 
        led23,
        led2,
        display, 
        THINGSPEAK_API_KEY, 
        readPot, 
        readDht
    );

    // Conexión a WiFi con funcion en telegram helper
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
