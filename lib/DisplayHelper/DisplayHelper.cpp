#include "DisplayHelper.h"

// Función de Inicialización
bool inicializarDisplay(U8G2_SSD1306_128X64_NONAME_F_SW_I2C &u8g2) {
    u8g2.begin();
    // Configuramos una fuente y un tamaño legible
    u8g2.setFont(u8g2_font_ncenB10_tr); 
    u8g2.setContrast(100);
    return true;
}

// Función de Mostrar en Display (Usando Page Loop)
void mostrarEnDisplay(U8G2_SSD1306_128X64_NONAME_F_SW_I2C &u8g2, String texto) {
    
    // Dividir el String por saltos de línea (\n)
    int lineStart = 0;
    int lineEnd = 0;
    int lineNumber = 0;
    
    // El Page Loop es obligatorio para U8G2: asegura la escritura
    u8g2.firstPage();
    do {
        u8g2.clearBuffer();
        lineStart = 0;
        lineNumber = 0;
        
        while (lineStart >= 0) {
            lineEnd = texto.indexOf('\n', lineStart);
            String line;

            if (lineEnd == -1) {
                line = texto.substring(lineStart);
                lineStart = -1; // Fin
            } else {
                line = texto.substring(lineStart, lineEnd);
                lineStart = lineEnd + 1;
            }
            
            // Dibujar línea: (x=0, y=Posición_Base + Línea*Separación)
            u8g2.drawStr(0, 16 + (lineNumber * 15), line.c_str());
            lineNumber++;
        }

    } while (u8g2.nextPage());
}