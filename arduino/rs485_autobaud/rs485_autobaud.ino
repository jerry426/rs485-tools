/*
 * RS485 Auto-Baud Detector - ESP32
 * Uses timing analysis to detect baud rate from actual data
 * Works with MAX485 module
 */

#include <HardwareSerial.h>

// MAX485 pins
#define RS485_RX_PIN 16     // RO → ESP32 RX
#define RS485_TX_PIN 17     // DI ← ESP32 TX (optional)
#define MAX485_RE_PIN 19    // RE - Active LOW
#define MAX485_DE_PIN 18    // DE - Active HIGH

HardwareSerial rs485(2);

// For auto-baud detection
volatile uint32_t lastEdgeTime = 0;
volatile uint32_t pulseWidths[256];
volatile int pulseCount = 0;
volatile bool measuring = false;

// Common baud rates
const unsigned long baudRates[] = {1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200};
const int numBaudRates = 8;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  
  pinMode(2, OUTPUT);
  pinMode(MAX485_RE_PIN, OUTPUT);
  pinMode(MAX485_DE_PIN, OUTPUT);
  
  digitalWrite(MAX485_RE_PIN, LOW);  // Enable receiver
  digitalWrite(MAX485_DE_PIN, LOW);  // Disable driver
  
  Serial.println("\n╔══════════════════════════════════════╗");
  Serial.println("║   RS485 AUTO-BAUD DETECTOR           ║");
  Serial.println("║   Uses timing analysis               ║");
  Serial.println("╚══════════════════════════════════════╝");
  Serial.println();
  Serial.println("Wiring:");
  Serial.println("  MAX485 RO → ESP32 GPIO16");
  Serial.println("  MAX485 RE → ESP32 GPIO19 (or GND)");
  Serial.println("  MAX485 DE → ESP32 GPIO18 (or GND)");
  Serial.println("  MAX485 VCC → 3.3V");
  Serial.println("  MAX485 GND → GND");
  Serial.println("  MAX485 A/B → RS485 bus");
  Serial.println();
  Serial.println("This tool will analyze signal timing");
  Serial.println("to automatically detect the baud rate.\n");
  
  showMenu();
}

void showMenu() {
  Serial.println("Commands:");
  Serial.println("  d - Detect baud rate (send data during this!)");
  Serial.println("  t - Test a specific baud rate");
  Serial.println("  s - Scan all common baud rates");
  Serial.println("  h - Show this menu");
  Serial.println();
}

// ISR for measuring pulse widths
void IRAM_ATTR measureTiming() {
  if (!measuring) return;
  
  uint32_t currentTime = micros();
  
  if (lastEdgeTime > 0) {
    uint32_t pulseWidth = currentTime - lastEdgeTime;
    
    // Filter reasonable values (1µs to 10ms)
    if (pulseWidth > 1 && pulseWidth < 10000) {
      if (pulseCount < 256) {
        pulseWidths[pulseCount++] = pulseWidth;
      }
    }
  }
  
  lastEdgeTime = currentTime;
}

void detectBaudRate() {
  Serial.println("\n╔══════════════════════════════════════╗");
  Serial.println("║   AUTO-BAUD DETECTION                ║");
  Serial.println("║   Send RS485 data now (10 seconds)   ║");
  Serial.println("╚══════════════════════════════════════╝");
  Serial.println();
  
  // Setup interrupt
  pulseCount = 0;
  lastEdgeTime = 0;
  measuring = true;
  
  attachInterrupt(digitalPinToInterrupt(RS485_RX_PIN), measureTiming, CHANGE);
  
  // Measure for 10 seconds
  unsigned long startTime = millis();
  while (millis() - startTime < 10000) {
    digitalWrite(2, !digitalRead(2));  // Blink while measuring
    delay(50);
    
    if (pulseCount > 50) {
      Serial.print("Detected ");
      Serial.print(pulseCount);
      Serial.println(" edges so far...");
    }
  }
  
  measuring = false;
  detachInterrupt(digitalPinToInterrupt(RS485_RX_PIN));
  
  digitalWrite(2, LOW);
  
  Serial.print("\nMeasurement complete. ");
  Serial.print(pulseCount);
  Serial.println(" edges captured.\n");
  
  if (pulseCount < 20) {
    Serial.println("⚠ Not enough edges detected");
    Serial.println("  No data on bus or very slow data rate");
    return;
  }
  
  // Analyze pulse widths
  Serial.println("Analyzing pulse widths...\n");
  
  // Find shortest pulse (1-bit time)
  uint32_t minPulse = 0xFFFFFFFF;
  uint32_t maxPulse = 0;
  uint64_t sumPulse = 0;
  
  for (int i = 0; i < pulseCount; i++) {
    if (pulseWidths[i] < minPulse) minPulse = pulseWidths[i];
    if (pulseWidths[i] > maxPulse) maxPulse = pulseWidths[i];
    sumPulse += pulseWidths[i];
  }
  
  uint32_t avgPulse = sumPulse / pulseCount;
  
  Serial.print("Shortest pulse: ");
  Serial.print(minPulse);
  Serial.println(" µs");
  
  Serial.print("Longest pulse:  ");
  Serial.print(maxPulse);
  Serial.println(" µs");
  
  Serial.print("Average pulse:  ");
  Serial.print(avgPulse);
  Serial.println(" µs");
  
  // Calculate baud rates
  Serial.println("\nEstimated baud rates:");
  
  // From shortest pulse (should be 1 bit time)
  unsigned long baudFromMin = 1000000 / minPulse;
  Serial.print("  From shortest pulse: ~");
  Serial.println(baudFromMin);
  
  // From average pulse
  unsigned long baudFromAvg = 1000000 / avgPulse;
  Serial.print("  From average pulse:  ~");
  Serial.println(baudFromAvg);
  
  // Find closest standard baud rate
  unsigned long closestBaud = baudRates[0];
  int minDiff = abs((long)baudFromMin - (long)closestBaud);
  
  for (int i = 1; i < numBaudRates; i++) {
    int diff = abs((long)baudFromMin - (long)baudRates[i]);
    if (diff < minDiff) {
      minDiff = diff;
      closestBaud = baudRates[i];
    }
  }
  
  Serial.print("\nClosest standard rate: ");
  Serial.println(closestBaud);
  
  // Test this baud rate
  Serial.print("\nTesting at ");
  Serial.print(closestBaud);
  Serial.println(" baud...\n");
  
  testBaudRate(closestBaud);
}

void testBaudRate(unsigned long baud) {
  rs485.begin(baud, SERIAL_8N1, RS485_RX_PIN, RS485_TX_PIN);
  
  Serial.print("Listening at ");
  Serial.print(baud);
  Serial.println(" baud for 5 seconds...\n");
  
  unsigned long startTime = millis();
  int bytesReceived = 0;
  
  while (millis() - startTime < 5000) {
    if (rs485.available()) {
      if (bytesReceived == 0) {
        Serial.println("✓ Data received!");
      }
      
      uint8_t byte = rs485.read();
      bytesReceived++;
      
      if (bytesReceived <= 16) {
        if (byte < 0x10) Serial.print("0");
        Serial.print(byte, HEX);
        Serial.print(" ");
      }
      
      digitalWrite(2, HIGH);
      delay(20);
      digitalWrite(2, LOW);
    }
  }
  
  if (bytesReceived > 0) {
    Serial.println("\n\n✓ SUCCESS!");
    Serial.print(bytesReceived);
    Serial.println(" bytes received at this baud rate");
    Serial.println("\nThis appears to be the correct baud rate!");
  } else {
    Serial.println("\n⚠ No data received at this rate");
    Serial.println("Try manual scan (press 's')");
  }
}

void scanAllBaudRates() {
  Serial.println("\n╔══════════════════════════════════════╗");
  Serial.println("║   SCANNING ALL BAUD RATES            ║");
  Serial.println("║   Send data continuously...          ║");
  Serial.println("╚══════════════════════════════════════╝\n");
  
  for (int i = 0; i < numBaudRates; i++) {
    unsigned long baud = baudRates[i];
    
    Serial.print("Testing ");
    Serial.print(baud);
    Serial.print(" baud: ");
    
    rs485.begin(baud, SERIAL_8N1, RS485_RX_PIN, RS485_TX_PIN);
    
    // Clear buffer
    while (rs485.available()) rs485.read();
    
    // Listen for 1 second
    unsigned long startTime = millis();
    bool gotData = false;
    int byteCount = 0;
    
    while (millis() - startTime < 1000) {
      if (rs485.available()) {
        rs485.read();
        byteCount++;
        gotData = true;
      }
    }
    
    if (gotData) {
      Serial.print("✓ GOT DATA (");
      Serial.print(byteCount);
      Serial.println(" bytes)");
      
      // Test this rate more thoroughly
      Serial.print("\nTesting ");
      Serial.print(baud);
      Serial.println(" baud more thoroughly...\n");
      
      testBaudRate(baud);
      return;
    } else {
      Serial.println("no data");
    }
  }
  
  Serial.println("\n⚠ No data at any standard baud rate");
  Serial.println("Check wiring or device activity");
}

void handleCommands() {
  if (Serial.available()) {
    char cmd = Serial.read();
    
    switch (cmd) {
      case 'd':
      case 'D':
        detectBaudRate();
        break;
        
      case 't':
      case 'T':
        {
          Serial.print("Enter baud rate to test: ");
          while (!Serial.available()) delay(10);
          unsigned long testBaud = Serial.parseInt();
          testBaudRate(testBaud);
        }
        break;
        
      case 's':
      case 'S':
        scanAllBaudRates();
        break;
        
      case 'h':
      case 'H':
        showMenu();
        break;
    }
    
    while (Serial.available()) Serial.read();
  }
}

void loop() {
  handleCommands();
  
  // Show statistics every 10 seconds
  static unsigned long lastStats = 0;
  if (millis() - lastStats > 10000) {
    Serial.print(".");
    lastStats = millis();
  }
  
  delay(10);
}