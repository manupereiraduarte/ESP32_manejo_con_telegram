#ifndef DISPLAYHELPER_H
#define DISPLAYHELPER_H

// 🚨 ¡CRÍTICO! U8g2lib.h DEBE estar aquí para que el compilador sepa qué es U8G2_...
#include <U8g2lib.h> 
#include <Arduino.h>

// Usa el tipo completo que definiste en main.cpp
void mostrarEnDisplay(U8G2_SSD1306_128X64_NONAME_F_SW_I2C &u8g2, String texto);
bool inicializarDisplay(U8G2_SSD1306_128X64_NONAME_F_SW_I2C &u8g2);

#endif