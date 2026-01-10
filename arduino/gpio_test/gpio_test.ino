/*
 * ESP32 GPIO Test - Verify pins work
 * Tests GPIO16 and GPIO17 by manually toggling and reading
 */

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  
  pinMode(2, OUTPUT);  // LED
  
  Serial.println("\n╔══════════════════════════════════════╗");
  Serial.println("║   GPIO PIN TEST - GPIO16 & GPIO17    ║");
  Serial.println("╚══════════════════════════════════════╝");
  Serial.println();
  Serial.println("This test will verify GPIO16 and GPIO17");
  Serial.println("are working correctly.");
  Serial.println();
  Serial.println("WIRING FOR THIS TEST:");
  Serial.println("  Connect GPIO16 to GPIO17 with jumper");
  Serial.println("  wire (same as loopback test)");
  Serial.println();
  Serial.println("Press any key to start test...\n");
  
  while (!Serial.available()) {
    digitalWrite(2, HIGH);
    delay(200);
    digitalWrite(2, LOW);
    delay(200);
  }
  Serial.read();
}

void loop() {
  Serial.println("Testing GPIO16 as OUTPUT, GPIO17 as INPUT");
  Serial.println("─────────────────────────────────────────");
  
  // Configure
  pinMode(16, OUTPUT);
  pinMode(17, INPUT_PULLUP);
  
  // Test 1: Set GPIO16 HIGH, read GPIO17
  digitalWrite(16, HIGH);
  delay(10);  // Let it settle
  
  bool read17 = digitalRead(17);
  Serial.print("GPIO16 = HIGH, GPIO17 reads: ");
  Serial.println(read17 ? "HIGH ✓" : "LOW ✗");
  
  digitalWrite(2, read17 ? HIGH : LOW);
  delay(500);
  
  // Test 2: Set GPIO16 LOW, read GPIO17
  digitalWrite(16, LOW);
  delay(10);
  
  read17 = digitalRead(17);
  Serial.print("GPIO16 = LOW, GPIO17 reads:  ");
  Serial.println(read17 ? "HIGH ✗" : "LOW ✓");
  
  digitalWrite(2, read17 ? HIGH : LOW);
  delay(500);
  
  Serial.println();
  
  // Now swap roles
  Serial.println("Testing GPIO17 as OUTPUT, GPIO16 as INPUT");
  Serial.println("─────────────────────────────────────────");
  
  pinMode(17, OUTPUT);
  pinMode(16, INPUT_PULLUP);
  
  // Test 3: Set GPIO17 HIGH, read GPIO16
  digitalWrite(17, HIGH);
  delay(10);
  
  bool read16 = digitalRead(16);
  Serial.print("GPIO17 = HIGH, GPIO16 reads: ");
  Serial.println(read16 ? "HIGH ✓" : "LOW ✗");
  
  digitalWrite(2, read16 ? HIGH : LOW);
  delay(500);
  
  // Test 4: Set GPIO17 LOW, read GPIO16
  digitalWrite(17, LOW);
  delay(10);
  
  read16 = digitalRead(16);
  Serial.print("GPIO17 = LOW, GPIO16 reads:  ");
  Serial.println(read16 ? "HIGH ✗" : "LOW ✓");
  
  digitalWrite(2, read16 ? HIGH : LOW);
  delay(500);
  
  Serial.println();
  Serial.println("╔══════════════════════════════════════╗");
  Serial.println("║   Test complete                      ║");
  Serial.println("║   Check results above                ║");
  Serial.println("╚══════════════════════════════════════╝");
  Serial.println();
  Serial.println("All readings should match (HIGH→HIGH, LOW→LOW)");
  Serial.println("Press 'r' to repeat, or check jumper wire!\n");
  
  // Wait for command
  while (!Serial.available()) delay(10);
  char cmd = Serial.read();
  if (cmd != 'r' && cmd != 'R') {
    Serial.println("Exiting test");
    while (1) delay(1000);
  }
  
  Serial.println("\n\nRepeating test...\n");
}