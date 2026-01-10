/*
 * RS485 Data Inspector - ESP32 with MAX485
 * Shows raw data as HEX and allows quick baud rate changes
 */

#include <HardwareSerial.h>

// MAX485 pins
#define RS485_RX_PIN 16
#define RS485_TX_PIN 17
#define MAX485_RE_PIN 19
#define MAX485_DE_PIN 18

HardwareSerial rs485(2);
unsigned long currentBaud = 4800;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  
  pinMode(2, OUTPUT);
  pinMode(MAX485_RE_PIN, OUTPUT);
  pinMode(MAX485_DE_PIN, OUTPUT);
  
  digitalWrite(MAX485_RE_PIN, LOW);  // Enable receiver
  digitalWrite(MAX485_DE_PIN, LOW);  // Disable driver
  
  initUART(currentBaud);
  
  showMenu();
}

void initUART(unsigned long baud) {
  rs485.begin(baud, SERIAL_8N1, RS485_RX_PIN, RS485_TX_PIN);
  rs485.setRxTimeout(2);
  Serial.print("\n✓ UART initialized at ");
  Serial.print(baud);
  Serial.println(" baud");
}

void showMenu() {
  Serial.println("\n╔══════════════════════════════════════╗");
  Serial.println("║   RS485 DATA INSPECTOR               ║");
  Serial.println("╠══════════════════════════════════════╣");
  Serial.println("║   Current Baud: 4800                 ║");
  Serial.println("║                                      ║");
  Serial.println("║   Commands:                          ║");
  Serial.println("║   1-9 - Common baud rates            ║");
  Serial.println("║   c   - Set custom baud              ║");
  Serial.println("║   h   - Show this menu               ║");
  Serial.println("╚══════════════════════════════════════╝");
}

void loop() {
  // LED heartbeat
  static unsigned long lastBlink = 0;
  if (millis() - lastBlink > 1000) {
    digitalWrite(2, !digitalRead(2));
    lastBlink = millis();
  }
  
  // Handle commands
  if (Serial.available()) {
    char cmd = Serial.read();
    
    switch (cmd) {
      case '1': currentBaud = 1200; initUART(currentBaud); break;
      case '2': currentBaud = 2400; initUART(currentBaud); break;
      case '3': currentBaud = 4800; initUART(currentBaud); break;
      case '4': currentBaud = 9600; initUART(currentBaud); break;
      case '5': currentBaud = 19200; initUART(currentBaud); break;
      case '6': currentBaud = 38400; initUART(currentBaud); break;
      case '7': currentBaud = 57600; initUART(currentBaud); break;
      case '8': currentBaud = 115200; initUART(currentBaud); break;
      case '9': currentBaud = 230400; initUART(currentBaud); break;
      case 'c':
      case 'C':
        Serial.print("Enter baud rate: ");
        while (!Serial.available()) delay(10);
        currentBaud = Serial.parseInt();
        initUART(currentBaud);
        break;
      case 'h':
      case 'H':
        showMenu();
        break;
    }
    
    while (Serial.available()) Serial.read();
  }
  
  // Check for RS485 data
  if (rs485.available()) {
    digitalWrite(2, HIGH);
    
    Serial.print("[");
    Serial.print(millis());
    Serial.print("ms] ");
    
    int bytesRead = 0;
    while (rs485.available() && bytesRead < 24) {
      uint8_t byte = rs485.read();
      
      // Always print as hex
      if (byte < 0x10) Serial.print("0");
      Serial.print(byte, HEX);
      Serial.print(" ");
      
      bytesRead++;
    }
    
    // If fewer than 8 bytes, try ASCII
    if (bytesRead < 8) {
      Serial.print("| '");
      // Re-read would need buffer, so just show we got data
      for (int i = 0; i < bytesRead; i++) {
        Serial.print(".");
      }
      Serial.print("'");
    }
    
    Serial.println();
    digitalWrite(2, LOW);
  }
}