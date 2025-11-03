#include "DisplayHelper.h"
#include "Wire.h" // Necesaria para I2C

// Define las dimensiones y la dirección (ajusta según tu pantalla)
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1 
#define SCREEN_ADDRESS 0x3C 

bool inicializarDisplay(Adafruit_SSD1306 &display) {
  // Inicialización I2C
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("Error al inicializar SSD1306"));
    return false;
  }
  display.display(); // Muestra el logo de Adafruit por un momento
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  return true;
}

void mostrarEnDisplay(Adafruit_SSD1306 &display, String texto) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(texto);
  display.display();
}