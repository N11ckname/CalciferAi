/*
 * Test OLED Ultra-Simple
 * Pour diagnostiquer les problèmes d'affichage
 */

#include <Wire.h>
#include <U8g2lib.h>

// Testez différentes configurations si la première ne marche pas

// Configuration 1 : SH1106 mode 2-page (économise RAM)
// U8G2_SH1106_128X64_NONAME_2_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// Si ça ne marche pas, commentez la ligne au-dessus et décommentez UNE des lignes ci-dessous :

// Configuration 2 : SSD1306 mode 2-page (économise RAM) - TEST EN COURS
U8G2_SSD1306_128X64_NONAME_2_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// Configuration 3 : SH1106 Software I2C
// U8G2_SH1106_128X64_NONAME_2_SW_I2C u8g2(U8G2_R0, A5, A4, U8X8_PIN_NONE);

// Configuration 4 : SSD1306 Software I2C
// U8G2_SSD1306_128X64_NONAME_2_SW_I2C u8g2(U8G2_R0, A5, A4, U8X8_PIN_NONE);

void setup() {
  Serial.begin(9600);
  delay(1000);
  
  Serial.println("=================================");
  Serial.println("   TEST OLED - DIAGNOSTIC");
  Serial.println("=================================");
  
  // Test I2C Scanner
  Serial.println("\n1. Scan bus I2C...");
  Wire.begin();
  scanI2C();
  
  // Initialisation écran
  Serial.println("\n2. Initialisation ecran...");
  if (!u8g2.begin()) {
    Serial.println("ERREUR: Impossible d'initialiser l'ecran!");
  } else {
    Serial.println("OK: Ecran initialise");
  }
  
  // Contraste au maximum
  u8g2.setContrast(255);
  Serial.println("3. Contraste regle a 255");
  
  Serial.println("\n=================================");
  Serial.println("Si vous voyez du texte sur l'ecran OLED:");
  Serial.println("  -> ECRAN OK!");
  Serial.println("\nSi ecran noir:");
  Serial.println("  -> Verifier adresse I2C dans scan");
  Serial.println("  -> Essayer autre configuration");
  Serial.println("=================================\n");
}

void loop() {
  static int count = 0;
  static unsigned long lastUpdate = 0;
  unsigned long now = millis();
  
  // Mise à jour toutes les 2 secondes
  if (now - lastUpdate >= 2000) {
    lastUpdate = now;
    count++;
    
    // Affichage en mode page
    u8g2.firstPage();
    do {
      u8g2.setFont(u8g2_font_10x20_tf);
      u8g2.drawStr(10, 25, "LUCIA");
      u8g2.setFont(u8g2_font_6x10_tf);
      u8g2.drawStr(10, 45, "Ecran OK!");
      
      char buf[16];
      sprintf(buf, "Test:%d", count);
      u8g2.drawStr(10, 60, buf);
    } while (u8g2.nextPage());
    
    Serial.print(".");
    if (count % 10 == 0) {
      Serial.println();
      Serial.print("Test iteration: ");
      Serial.println(count);
    }
  }
}

void scanI2C() {
  byte error, address;
  int nDevices = 0;
  
  Serial.println("Recherche peripheriques I2C...");
  
  for(address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    
    if (error == 0) {
      Serial.print("Peripherique trouve a l'adresse 0x");
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);
      
      // Identification du périphérique
      if (address == 0x3C) {
        Serial.print(" (OLED SSD1306/SH1106)");
      } else if (address == 0x3D) {
        Serial.print(" (OLED alternatif)");
      }
      Serial.println();
      nDevices++;
    }
  }
  
  if (nDevices == 0) {
    Serial.println("AUCUN peripherique I2C trouve!");
    Serial.println("\nVERIFIER:");
    Serial.println("  - Connexion SDA (A4)");
    Serial.println("  - Connexion SCL (A5)");
    Serial.println("  - Alimentation VCC (5V)");
    Serial.println("  - Connexion GND");
  } else {
    Serial.print("Nombre de peripheriques trouves: ");
    Serial.println(nDevices);
  }
}

