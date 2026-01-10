/*
 * RS485 ESPHome Monitor - ESP32 with MAX485
 * Dedicated sniffer for 4800 baud, 8-O-1 configuration
 * Shows both frame view AND continuous stream view
 */

#include <HardwareSerial.h>

// MAX485 module connections
#define RS485_RX_PIN 16     // RO → ESP32 GPIO16 (RX)
#define RS485_TX_PIN 17     // DI ← ESP32 GPIO17 (TX) - optional
#define MAX485_RE_PIN 19    // RE - Active LOW, connect to GND or GPIO19
#define MAX485_DE_PIN 18    // DE - Active HIGH, connect to GND (listening mode)

// Configuration - matches your ESPHome device
#define BAUD_RATE 4800      // Confirmed 4800 baud
#define DATA_BITS 8         // 8 data bits
#define PARITY_MODE SERIAL_PARITY_ODD  // ODD parity (critical!)
#define STOP_BITS 1         // 1 stop bit

HardwareSerial rs485(2);

// Statistics
unsigned long totalBytes = 0;
unsigned long totalFrames = 0;
unsigned long lastFrameTime = 0;
unsigned long startTime = 0;

// Stream mode - continuous single line display
bool streamMode = false;
unsigned long lastStreamByte = 0;
#define STREAM_TIMEOUT 100  // Start new line after 100ms gap

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  
  pinMode(2, OUTPUT);  // Built-in LED
  pinMode(MAX485_RE_PIN, OUTPUT);
  pinMode(MAX485_DE_PIN, OUTPUT);
  
  // Enable receiver, disable driver (listening mode)
  digitalWrite(MAX485_RE_PIN, LOW);   // RE=LOW enables receiver
  digitalWrite(MAX485_DE_PIN, LOW);   // DE=LOW disables driver
  
  Serial.println("\n╔══════════════════════════════════════╗");
  Serial.println("║   ESPHOME RS485 MONITOR              ║");
  Serial.println("╠══════════════════════════════════════╣");
  Serial.println("║   Device: Modbus/RS485 Sensor        ║");
  Serial.print("║   Baud: ");
  Serial.print(BAUD_RATE);
  Serial.println("                         ║");
  Serial.println("║   Parity: ODD                        ║");
  Serial.println("║   Data: 8 bits                       ║");
  Serial.println("║   Stop: 1 bit                        ║");
  Serial.println("╚══════════════════════════════════════╝");
  Serial.println();
  Serial.println("Hardware:");
  Serial.print("  MAX485 RO  → ESP32 GPIO");
  Serial.println(RS485_RX_PIN);
  Serial.print("  MAX485 RE  → ESP32 GPIO");
  Serial.println(MAX485_RE_PIN);
  Serial.print("  MAX485 DE  → ESP32 GPIO");
  Serial.println(MAX485_DE_PIN);
  Serial.println("  MAX485 VCC → 3.3V");
  Serial.println("  MAX485 GND → GND");
  Serial.println("  MAX485 A/B → RS485 bus");
  Serial.println();
  
  Serial.println("Initializing UART...");
  initUART();
  
  Serial.println("\n╔══════════════════════════════════════╗");
  Serial.println("║   MONITORING ESPHOME DEVICE          ║");
  Serial.println("║   Waiting for data...                ║");
  Serial.println("╚══════════════════════════════════════╝");
  Serial.println();
  
  startTime = millis();
  lastStreamByte = millis();
}

void initUART() {
  // Configure UART with correct parity
  rs485.begin(BAUD_RATE, SERIAL_8O1, RS485_RX_PIN, RS485_TX_PIN);
  // Note: SERIAL_8O1 = 8 data, ODD parity, 1 stop
  
  Serial.print("✓ UART configured: ");
  Serial.print(BAUD_RATE);
  Serial.print(" baud, ODD parity, 8-O-1");
  Serial.println();
  
  // Verify pin state
  Serial.print("  RX pin state: ");
  bool pinState = digitalRead(RS485_RX_PIN);
  Serial.println(pinState ? "HIGH" : "LOW");
  
  if (pinState) {
    Serial.println("  ⚠ WARNING: RX is HIGH (bus may be idle)");
  } else {
    Serial.println("  ✓ RX is LOW (good idle state)");
  }
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
    
    if (cmd == 's' || cmd == 'S') {
      streamMode = !streamMode;
      Serial.print("\n\nStream mode: ");
      Serial.println(streamMode ? "ENABLED" : "DISABLED");
      if (streamMode) {
        Serial.println("Data will be shown continuously on one line");
        Serial.println("New line starts after 100ms gap\n");
      } else {
        Serial.println("Showing frames only\n");
      }
    } else if (cmd == 'c' || cmd == 'C') {
      totalBytes = 0;
      totalFrames = 0;
      Serial.println("\nCounters cleared\n");
    }
    
    while (Serial.available()) Serial.read();
  }
  
  // Capture RS485 data
  if (rs485.available()) {
    digitalWrite(2, HIGH);  // LED ON when receiving
    
    // Handle stream mode - continuous single line
    if (streamMode) {
      // Start new line if gap > 100ms
      if (millis() - lastStreamByte > STREAM_TIMEOUT) {
        if (totalBytes > 0) {
          Serial.println();  // End previous line
        }
        Serial.print("[Stream] ");
      }
      
      // Print bytes continuously
      while (rs485.available() && totalBytes < 10000) {
        uint8_t byte = rs485.read();
        
        if (byte < 0x10) Serial.print("0");
        Serial.print(byte, HEX);
        Serial.print(" ");
        
        totalBytes++;
        lastStreamByte = millis();
      }
    }
    
    // Handle frame mode - grouped by timing
    else {
      totalFrames++;
      lastFrameTime = millis();
      
      Serial.print("[");
      Serial.print(millis() / 1000);
      Serial.print(".");
      int ms = millis() % 1000;
      if (ms < 100) Serial.print("0");
      if (ms < 10) Serial.print("0");
      Serial.print(ms);
      Serial.print("] Frame #");
      Serial.print(totalFrames);
      Serial.print(" - ");
      
      // Read all bytes in this frame
      int byteCount = 0;
      uint8_t frameBytes[64];
      
      while (rs485.available() && byteCount < 64) {
        frameBytes[byteCount] = rs485.read();
        byteCount++;
        totalBytes++;
      }
      
      Serial.print(byteCount);
      Serial.print(" bytes: ");
      
      // Print HEX
      for (int i = 0; i < byteCount; i++) {
        if (frameBytes[i] < 0x10) Serial.print("0");
        Serial.print(frameBytes[i], HEX);
        Serial.print(" ");
      }
      
      // Align columns
      for (int i = byteCount; i < 16; i++) {
        Serial.print("   ");
      }
      Serial.print("| ");
      
      // Decode frame
      decodeFrame(frameBytes, byteCount);
      
      Serial.println();
    }
    
    delay(10);
    digitalWrite(2, LOW);
  }
  
  }
}

void decodeFrame(uint8_t* bytes, int count) {
  // Check for Modbus RTU pattern
  if (count >= 4) {
    uint8_t slaveId = bytes[0];
    uint8_t functionCode = bytes[1];
    
    if (slaveId >= 1 && slaveId <= 247) {
      Serial.print("Modbus:");
      
      // Slave ID
      Serial.print(" ID=");
      Serial.print(slaveId);
      
      // Function code
      Serial.print(" FC=0x");
      if (functionCode < 0x10) Serial.print("0");
      Serial.print(functionCode, HEX);
      Serial.print(" ");
      
      // Function name
      switch (functionCode) {
        case 0x01: Serial.print("(Read Coils)"); break;
        case 0x02: Serial.print("(Read Discrete)"); break;
        case 0x03: Serial.print("(Read Holding)"); break;
        case 0x04: Serial.print("(Read Input)"); break;
        case 0x05: Serial.print("(Write Coil)"); break;
        case 0x06: Serial.print("(Write Register)"); break;
        case 0x10: Serial.print("(Write Multiple)"); break;
        default: Serial.print("(Unknown)"); break;
      }
      
      // Data bytes
      if (count > 3) {
        Serial.print(" Data=");
        int dataBytes = count - 3;  // ID + FC + DATA + CRC(2)
        if (dataBytes > 0) {
          int startIdx = 2;
          int dataLen = min(dataBytes, count - 2);
          for (int i = startIdx; i < startIdx + dataLen - 2 && i < count; i++) {
            if (bytes[i] < 0x10) Serial.print("0");
            Serial.print(bytes[i], HEX);
            Serial.print(" ");
          }
        }
      }
      return;
    }
  }
  
  // ASCII interpretation
  bool hasAscii = false;
  for (int i = 0; i < count; i++) {
    if (bytes[i] >= 32 && bytes[i] <= 126) {
      hasAscii = true;
      break;
    }
  }
  
  if (hasAscii) {
    Serial.print("ASCII: \"");
    for (int i = 0; i < count && i < 16; i++) {
      if (bytes[i] >= 32 && bytes[i] <= 126) {
        Serial.print((char)bytes[i]);
      } else {
        Serial.print("·");
      }
    }
    Serial.print("\"");
  } else {
    Serial.print("Binary (");
    Serial.print(count);
    Serial.print(" bytes)");
  }
}

void printStatistics() {
  unsigned long uptime = (millis() - startTime) / 1000;
  
  Serial.println();
  Serial.println("╔══════════════════════════════════════╗");
  Serial.println("║   STATISTICS                         ║");
  Serial.println("╠══════════════════════════════════════╣");
  Serial.print("║   Uptime:     ");
  Serial.print(uptime);
  Serial.println("s                     ║");
  Serial.print("║   Frames:     ");
  Serial.print(totalFrames);
  Serial.println("                       ║");
  Serial.print("║   Bytes:      ");
  Serial.print(totalBytes);
  Serial.println("                       ║");
  if (totalFrames > 0) {
    Serial.print("║   Avg/Frame:  ");
    Serial.print(totalBytes / totalFrames, 1);
    Serial.println(" bytes                 ║");
  }
  Serial.print("║   Mode:       ");
  Serial.print(streamMode ? "Stream" : "Frames");
  Serial.println("                ║");
  Serial.println("╚══════════════════════════════════════╝");
  Serial.println();
  Serial.println("Commands: s=toggle stream, c=clear, h=help");
  Serial.println();
}