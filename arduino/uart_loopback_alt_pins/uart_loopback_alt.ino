/*
 * ESP32 UART Loopback Test - ALTERNATIVE PINS
 * Tests UART2 on GPIO4 (TX) and GPIO25 (RX)
 * Connect GPIO4 to GPIO25 with jumper wire
 */

#include <HardwareSerial.h>

// ALTERNATIVE UART2 pins - try these instead
#define TX_PIN 4    // Connect to GPIO25
#define RX_PIN 25   // Connect to GPIO4

HardwareSerial testSerial(2);
unsigned long testCount = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  
  pinMode(2, OUTPUT);
  
  Serial.println("\n╔══════════════════════════════════════╗");
  Serial.println("║   UART2 LOOPBACK - ALT PINS          ║");
  Serial.println("╠══════════════════════════════════════╣");
  Serial.println("║   Testing GPIO4 (TX) and GPIO25 (RX) ║");
  Serial.println("║   Connect GPIO4 to GPIO25 with wire  ║");
  Serial.println("╚══════════════════════════════════════╝");
  Serial.println();
  Serial.println("WIRING:");
  Serial.println("  GPIO4 (TX) ───┐");
  Serial.println("                ├──► Jumper wire");
  Serial.println("  GPIO25 (RX) ──┘");
  Serial.println();
  Serial.println("Press any key to start test...\n");
  
  while (!Serial.available()) {
    digitalWrite(2, HIGH);
    delay(100);
    digitalWrite(2, LOW);
    delay(100);
  }
  Serial.read();
  
  Serial.println("Starting UART2 on GPIO4/25...\n");
  
  pinMode(TX_PIN, OUTPUT);
  pinMode(RX_PIN, INPUT_PULLUP);
  
  testSerial.begin(4800, SERIAL_8N1, RX_PIN, TX_PIN);
  
  delay(100);
  
  Serial.println("UART2 initialized at 4800 baud");
  Serial.print("GPIO");
  Serial.print(TX_PIN);
  Serial.print(" (TX) state: ");
  Serial.println(digitalRead(TX_PIN) ? "HIGH" : "LOW");
  
  Serial.print("GPIO");
  Serial.print(RX_PIN);
  Serial.print(" (RX) state: ");
  Serial.println(digitalRead(RX_PIN) ? "HIGH" : "LOW");
  
  Serial.println("\n╔══════════════════════════════════════╗");
  Serial.println("║   TESTING LOOPBACK                   ║");
  Serial.println("║   Sending test data...               ║");
  Serial.println("╚══════════════════════════════════════╝\n");
}

void loop() {
  static unsigned long lastSend = 0;
  
  if (millis() - lastSend > 2000) {
    testCount++;
    
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "Alt Pin Test #%lu!\r\n", testCount);
    
    Serial.print("Sending: ");
    Serial.print(buffer);
    
    testSerial.print(buffer);
    
    digitalWrite(2, HIGH);
    delay(50);
    digitalWrite(2, LOW);
    
    lastSend = millis();
    delay(100);  // Allow time for loopback
  }
  
  if (testSerial.available()) {
    digitalWrite(2, HIGH);
    
    Serial.print("Received: ");
    
    while (testSerial.available()) {
      char c = testSerial.read();
      Serial.print(c);
    }
    
    Serial.println(" ✓ WORKING!");
    digitalWrite(2, LOW);
  }
  
  delay(1);
}