/*
 * RS485 Wiring Diagnostic Tool for ESP32
 * This tool helps verify your XY-485 module connections
 */

#include <HardwareSerial.h>

// Pin configuration
#define RS485_RX_PIN 16      // GPIO16 - connect to XY-485 RX
#define RS485_TX_PIN 17      // GPIO17 - connect to XY-485 TX
#define LED_PIN 2            // Built-in LED

// Use UART2 for diagnostics
HardwareSerial rs485(2);

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  
  pinMode(LED_PIN, OUTPUT);
  pinMode(RS485_RX_PIN, INPUT_PULLUP);
  pinMode(RS485_TX_PIN, OUTPUT);
  
  // Initial LED blink to show it's running
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
    delay(100);
  }
  
  Serial.println("\n╔══════════════════════════════════════╗");
  Serial.println("║  RS485 WIRING DIAGNOSTIC TOOL        ║");
  Serial.println("╚══════════════════════════════════════╝");
  Serial.println("\nThis tool will help verify your XY-485 connections");
  Serial.println("even without an active RS485 bus.\n");
  
  runDiagnostics();
}

void runDiagnostics() {
  // Test 1: Check power and ground
  Serial.println("TEST 1: Power and Ground");
  Serial.println("─────────────────────────");
  
  Serial.print("RX pin voltage (should be 3.3V): ");
  int rxReading = analogRead(RS485_RX_PIN);
  float rxVoltage = (rxReading * 3.3) / 4095.0;
  Serial.print(rxVoltage, 2);
  Serial.println("V");
  
  if (rxVoltage < 0.5) {
    Serial.println("⚠ LOW VOLTAGE - Check XY-485 power!");
    Serial.println("  → XY-485 VCC should be connected to ESP32 3V3");
    Serial.println("  → XY-485 GND should be connected to ESP32 GND");
  } else if (rxVoltage > 2.5 && rxVoltage < 3.6) {
    Serial.println("✓ Voltage looks good");
  } else {
    Serial.println("⚠ UNUSUAL VOLTAGE - Check connections");
  }
  
  delay(1000);
  
  // Test 2: Check idle state
  Serial.println("\nTEST 2: RS485 Bus Idle State");
  Serial.println("─────────────────────────────");
  
  Serial.print("RX pin digital state: ");
  bool rxState = digitalRead(RS485_RX_PIN);
  Serial.println(rxState ? "HIGH" : "LOW");
  
  if (rxState == HIGH) {
    Serial.println("⚠ RX pin is HIGH (idle state should be LOW)");
    Serial.println("  Possible causes:");
    Serial.println("  1. XY-485 module is not powered");
    Serial.println("  2. No ground connection to RS485 bus");
    Serial.println("  3. A/B wires are disconnected");
    Serial.println("  4. RS485 bus needs termination resistor");
  } else {
    Serial.println("✓ RX pin is LOW (correct idle state)");
  }
  
  delay(1000);
  
  // Test 3: Listen for any activity
  Serial.println("\nTEST 3: Signal Activity Monitor");
  Serial.println("────────────────────────────────");
  Serial.println("Monitoring for 5 seconds...\n");
  
  rs485.begin(4800, SERIAL_8N1, RS485_RX_PIN, RS485_TX_PIN);
  
  unsigned long startTime = millis();
  int transitions = 0;
  int bytesReceived = 0;
  bool lastState = digitalRead(RS485_RX_PIN);
  
  while (millis() - startTime < 5000) {
    // Count state transitions
    bool currentState = digitalRead(RS485_RX_PIN);
    if (currentState != lastState) {
      transitions++;
      lastState = currentState;
      digitalWrite(LED_PIN, currentState);
    }
    
    // Check for UART data
    if (rs485.available()) {
      uint8_t byte = rs485.read();
      bytesReceived++;
      Serial.print("  Data received: 0x");
      if (byte < 0x10) Serial.print("0");
      Serial.println(byte, HEX);
      digitalWrite(LED_PIN, HIGH);
    }
    
    delay(1);
  }
  
  digitalWrite(LED_PIN, LOW);
  
  Serial.print("\n  State transitions: ");
  Serial.println(transitions);
  Serial.print("  Bytes received: ");
  Serial.println(bytesReceived);
  
  if (transitions > 10) {
    Serial.println("\n✓ Signal activity detected!");
    Serial.println("  Bus is active - try the listener sketch");
  } else if (transitions > 0) {
    Serial.println("\n⚠ Some noise detected");
    Serial.println("  May be floating inputs or weak signal");
  } else {
    Serial.println("\n⚠ No activity detected");
    Serial.println("  Bus may be idle or disconnected");
  }
  
  delay(1000);
  
  // Test 4: Try different baud rates quickly
  Serial.println("\nTEST 4: Quick Baud Rate Test");
  Serial.println("─────────────────────────────");
  
  const unsigned long testRates[] = {1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200};
  const int numRates = sizeof(testRates) / sizeof(testRates[0]);
  
  for (int i = 0; i < numRates; i++) {
    Serial.print("  Testing ");
    Serial.print(testRates[i]);
    Serial.print(" baud: ");
    
    rs485.begin(testRates[i], SERIAL_8N1, RS485_RX_PIN, RS485_TX_PIN);
    delay(50);  // Stabilize
    
    // Clear buffer
    while (rs485.available()) rs485.read();
    
    // Listen briefly
    unsigned long listenStart = millis();
    int bytesAtRate = 0;
    
    while (millis() - listenStart < 100) {
      if (rs485.available()) {
        rs485.read();
        bytesAtRate++;
      }
    }
    
    if (bytesAtRate > 0) {
      Serial.print(bytesAtRate);
      Serial.println(" bytes ✓");
    } else {
      Serial.println("no data");
    }
  }
  
  delay(1000);
  
  // Summary
  Serial.println("\n╔══════════════════════════════════════╗");
  Serial.println("║  DIAGNOSTIC SUMMARY                  ║");
  Serial.println("╠══════════════════════════════════════╣");
  Serial.println("║  If you see data in Test 3 or 4:     ║");
  Serial.println("║  → Use rs485_listener.ino sketch     ║");
  Serial.println("║                                      ║");
  Serial.println("║  If NO data anywhere:                ║");
  Serial.println("║  → Check wiring (see below)          ║");
  Serial.println("║  → Verify devices are communicating  ║");
  Serial.println("║                                      ║");
  Serial.println("║  CRITICAL WIRING CHECKLIST:          ║");
  Serial.println("║  ✓ XY-485 VCC → ESP32 3V3            ║");
  Serial.println("║  ✓ XY-485 GND → ESP32 GND → RS485 GND║");
  Serial.println("║  ✓ XY-485 RX  → ESP32 GPIO16         ║");
  Serial.println("║  ✓ XY-485 A+  → RS485 A              ║");
  Serial.println("║  ✓ XY-485 B-  → RS485 B              ║");
  Serial.println("╚══════════════════════════════════════╝");
  Serial.println("\nPress 'r' to repeat diagnostics");
}

void loop() {
  if (Serial.available()) {
    char cmd = Serial.read();
    if (cmd == 'r' || cmd == 'R') {
      Serial.println("\n\n");
      runDiagnostics();
    }
    while (Serial.available()) Serial.read();
  }
  
  delay(100);
}