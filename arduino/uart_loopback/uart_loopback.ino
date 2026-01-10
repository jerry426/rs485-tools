/*
 * ESP32 UART Loopback Test
 * Connect ESP32 GPIO16 (TX) to GPIO17 (RX) with a jumper wire
 * This tests if UART2 is working without any RS485 hardware
 */

#include <HardwareSerial.h>

// UART2 pins
#define TX_PIN 16  // Connect this to GPIO17 with jumper wire
#define RX_PIN 17  // Connect this to GPIO16 with jumper wire

HardwareSerial testSerial(2);
unsigned long testCount = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  
  pinMode(2, OUTPUT);  // Built-in LED
  
  Serial.println("\n╔══════════════════════════════════════╗");
  Serial.println("║   ESP32 UART2 LOOPBACK TEST          ║");
  Serial.println("╠══════════════════════════════════════╣");
  Serial.println("║   Connect GPIO16 to GPIO17 with      ║");
  Serial.println("║   a jumper wire for loopback test    ║");
  Serial.println("╚══════════════════════════════════════╝");
  Serial.println();
  Serial.println("WIRING:");
  Serial.println("  GPIO16 (TX) ───┐");
  Serial.println("                 ├──► Jumper wire");
  Serial.println("  GPIO17 (RX) ───┘");
  Serial.println();
  Serial.println("Press any key when ready to start test...\n");
  
  // Wait for user
  while (!Serial.available()) {
    digitalWrite(2, HIGH);
    delay(100);
    digitalWrite(2, LOW);
    delay(100);
  }
  Serial.read();  // Clear the character
  
  Serial.println("Starting UART2...\n");
  
  // Configure pins
  pinMode(TX_PIN, OUTPUT);
  pinMode(RX_PIN, INPUT_PULLUP);
  
  // Initialize UART2
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
  Serial.println("║   LOOPBACK TEST ACTIVE               ║");
  Serial.println("║   Sending test data...               ║");
  Serial.println("╚══════════════════════════════════════╝\n");
}

void loop() {
  static unsigned long lastSend = 0;
  
  // Send test data every 2 seconds
  if (millis() - lastSend > 2000) {
    testCount++;
    
    // Create test message
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "Test #%lu: Hello UART2!\r\n", testCount);
    
    Serial.print("Sending: ");
    Serial.print(buffer);
    
    // Send via UART2
    testSerial.print(buffer);
    
    digitalWrite(2, HIGH);
    delay(50);
    digitalWrite(2, LOW);
    
    lastSend = millis();
    
    // Allow time for transmission
    delay(100);
  }
  
  // Check for received data
  if (testSerial.available()) {
    digitalWrite(2, HIGH);
    
    Serial.print("Received: ");
    
    // Read all available data
    while (testSerial.available()) {
      char c = testSerial.read();
      Serial.print(c);
    }
    
    Serial.println(" ✓ LOOPBACK WORKING!");
    digitalWrite(2, LOW);
  }
  
  // Status every 10 seconds if no data
  static unsigned long lastStatus = 0;
  if (millis() - lastStatus > 10000 && !testSerial.available()) {
    Serial.print(".");
    lastStatus = millis();
  }
}