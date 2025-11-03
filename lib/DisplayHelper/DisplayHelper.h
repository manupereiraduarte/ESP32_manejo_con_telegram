#ifndef DISPLAYHELPER_H
#define DISPLAYHELPER_H

#include <Adafruit_SSD1306.h>
#include <Arduino.h>

// Declaración de una función para inicializar la pantalla
bool inicializarDisplay(Adafruit_SSD1306 &display); 

// La función que ya tienes
void mostrarEnDisplay(Adafruit_SSD1306 &display, String texto);

#endif