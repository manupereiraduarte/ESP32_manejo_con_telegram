#ifndef DISPLAYHELPER_H
#define DISPLAYHELPER_H

#include <Arduino.h>

#include <U8g2lib.h> // 🚨 Cambiar librería

// La función ahora recibe una referencia al objeto U8G2
void mostrarEnDisplay(U8G2_SSD1306_128X64_NONAME_F_SW_I2C &u8g2, String texto);
bool inicializarDisplay(U8G2_SSD1306_128X64_NONAME_F_SW_I2C &u8g2);

#endif