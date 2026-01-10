/*
 * RS485 ESPHome Monitor - Clean Version
 * 4800 baud, ODD parity, 8-O-1 configuration
 * MAX485 module support
 * Shows both frame view and stream view (single line)
 * No automatic statistics (manual with 'i' command)
 */

#include <HardwareSerial.h>

// MAX485 connections
#define RS485_RX_PIN 16
#define MAX485_RE_PIN 19
#define MAX485_DE_PIN 18

// Configuration
#define BAUD_RATE 4800
HardwareSerial rs485(2);

// Statistics
unsigned long totalBytes = 0;
unsigned long totalFrames = 0;
unsigned long startTime = 0;

// Modes
bool streamMode = false;
unsigned long lastStreamByte = 0;
#define STREAM_TIMEOUT 100

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  
  pinMode(2, OUTPUT);
  pinMode(MAX485_RE_PIN, OUTPUT);
  pinMode(MAX485_DE_PIN, OUTPUT);
  
  digitalWrite(MAX485_RE_PIN, LOW);
  digitalWrite(MAX485_DE_PIN, LOW);
  
  Serial.println("\n╔══════════════════════════════════════╗");
  Serial.println("║   ESPHOME RS485 MONITOR v2.0         ║");
  Serial.println("╠══════════════════════════════════════╣");
  Serial.print("║   Baud: ");
  Serial.print(BAUD_RATE);
  Serial.println("                         ║");
  Serial.println("║   Parity: ODD                        ║");
  Serial.println("║   Mode: Frames (press 's' for stream)║");
  Serial.println("╚══════════════════════════════════════╝");
  Serial.println();
  
  // Configure UART
  rs485.begin(BAUD_RATE, SERIAL_8O1, RS485_RX_PIN, -1);
  
  Serial.println("Commands:");
  Serial.println("  s - Toggle stream mode (single line)");
  Serial.println("  i - Show statistics");
  Serial.println("  c - Clear counters");
  Serial.println("  h - Show help");
  Serial.println();
  
  startTime = millis();
  Serial.println("Listening...\n");
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
        Serial.println("Data shown continuously on one line");
        Serial.println("New line after 100ms gap\n");
      } else {
        Serial.println("Showing frames only\n");
      }
    } else if (cmd == 'i' || cmd == 'I') {
      printStatistics();
    } else if (cmd == 'c' || cmd == 'C') {
      totalBytes = 0;
      totalFrames = 0;
      Serial.println("\nCounters cleared\n");
    } else if (cmd == 'h' || cmd == 'H') {
      showHelp();
    }
    
    while (Serial.available()) Serial.read();
  }
  
  // Capture RS485 data
  if (rs485.available()) {
    digitalWrite(2, HIGH);
    
    if (streamMode) {
      // Stream mode - single line continuous
      if (millis() - lastStreamByte > STREAM_TIMEOUT) {
        if (totalBytes > 0) Serial.println();
        Serial.print("[Stream] ");
      }
      
      while (rs485.available() && totalBytes < 10000) {
        uint8_t byte = rs485.read();
        if (byte < 0x10) Serial.print("0");
        Serial.print(byte, HEX);
        Serial.print(" ");
        totalBytes++;
        lastStreamByte = millis();
      }
    } else {
      // Frame mode - grouped by timing
      totalFrames++;
      
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
      
      // Align and decode
      for (int i = byteCount; i < 16; i++) Serial.print("   ");
      Serial.print("| ");
      decodeFrame(frameBytes, byteCount);
      
      Serial.println();
    }
    
    delay(10);
    digitalWrite(2, LOW);
  }
}

void decodeFrame(uint8_t* bytes, int count) {
  if (count >= 4 && bytes[0] >= 1 && bytes[0] <= 247) {
    uint8_t slaveId = bytes[0];
    uint8_t functionCode = bytes[1];
    
    Serial.print("Modbus: ID=");
    Serial.print(slaveId);
    Serial.print(" FC=0x");
    if (functionCode < 0x10) Serial.print("0");
    Serial.print(functionCode, HEX);
    Serial.print(" ");
    
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
    return;
  }
  
  // ASCII check
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
}

void showHelp() {
  Serial.println("\n╔══════════════════════════════════════╗");
  Serial.println("║   HELP - Available Commands          ║");
  Serial.println("╠══════════════════════════════════════╣");
  Serial.println("║  s - Toggle stream mode              ║");
  Serial.println("║     (single line continuous output)  ║");
  Serial.println("║                                      ║");
  Serial.println("║  i - Show statistics                 ║");
  Serial.println("║     (uptime, frames, bytes, avg)     ║");
  Serial.println("║                                      ║");
  Serial.println("║  c - Clear counters                  ║");
  Serial.println("║     (reset frame and byte counts)    ║");
  Serial.println("║                                      ║");
  Serial.println("║  h - Show this help                  ║");
  Serial.println("╚══════════════════════════════════════╝");
  Serial.println();
}