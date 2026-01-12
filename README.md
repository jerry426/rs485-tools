# RS485 Tools

A collection of RS485 utilities for Arduino/ESP32 development and debugging.

## Main Utility

**📍 Location: `arduino/rs485_sniffer/rs485_sniffer/`**

The `rs485_sniffer` is the primary utility in this project. It's an RS485 communication monitor for ESPHome devices that captures and displays RS485 data frames in real-time.

### Quick Start

1. Navigate to the main utility: `cd arduino/rs485_sniffer/rs485_sniffer/`
2. Open `rs485_sniffer.ino` in Arduino IDE
3. Upload to your ESP board
4. See [arduino/rs485_sniffer/readme.md](arduino/rs485_sniffer/readme.md) for detailed documentation

## Other Utilities

This repository also contains supporting utilities in the `arduino/` directory:

| Utility | Purpose |
|---------|---------|
| `rs485_autobaud/` | Auto-detection of RS485 baud rates |
| `rs485_data_inspector/` | Detailed RS485 data inspection |
| `rs485_esphome_stream/` | ESPHome streaming utilities |
| `rs485_wiring_diagnostic/` | Wiring and hardware diagnostics |
| `gpio_test/` | GPIO pin testing |
| `uart_loopback/`, `uart_loopback_test/`, `uart_loopback_alt_pins/` | UART loopback testing |

These utilities are provided for development and debugging purposes. The main production utility is **rs485_sniffer**.

---

# RS485 Sniffer - ESP32

An intelligent RS485 bus sniffer that can auto-detect baud rates and capture traffic using an ESP32 and XY-485 converter module.

## Features

- **Automatic Baud Rate Detection**: Analyzes RS485 traffic to automatically determine baud rate
- **Multiple Baud Rate Support**: 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200
- **Non-intrusive Monitoring**: Passive listening without interfering with the bus
- **Real-time Capture**: Displays data as hex and ASCII with timestamps
- **Ring Buffer**: 4KB capture buffer prevents data loss during bursts
- **Interactive Controls**: Commands to detect, set, and manage capture

## Hardware Requirements

- ESP32 Development Board (any variant with UART2 available)
- XY-485 TTL to RS485 Converter Module
- RS485 bus to monitor (A, B, GND lines)
- Breadboard and jumper wires

## Wiring Connections

### ESP32 to XY-485 (TTL Side)

| ESP32 Pin | XY-485 Pin | Purpose |
|-----------|------------|---------|
| 3V3       | VCC        | Power (3.3V) |
| GND       | GND        | Ground |
| GPIO16    | RX         | RS485 Data Input to ESP32 |
| GPIO17    | TX         | RS485 Data Output (optional) |

### XY-485 to RS485 Bus

| XY-485 Pin | RS485 Bus | Purpose |
|------------|-----------|---------|
| A+         | A         | Differential Positive |
| B-         | B         | Differential Negative |
| GND        | GND       | Ground Reference |

**Important Notes:**
- This setup monitors the bus passively - the TX pin is connected but not actively used for sniffing
- Some XY-485 modules have DE/RE pins for direction control. These can be left disconnected for passive monitoring
- Ensure common ground between the RS485 bus and ESP32
- The XY-485 handles 3.3V to RS485 level conversion automatically

## Installation

### Arduino IDE

1. Open Arduino IDE
2. Select your ESP32 board from Tools > Board
3. Select the correct COM port
4. Open `arduino/rs485_sniffer/rs485_sniffer/rs485_sniffer.ino`
5. Click Upload

### PlatformIO

1. Create a new PlatformIO project for ESP32
2. Copy `arduino/rs485_sniffer/rs485_sniffer/rs485_sniffer.ino` to `src/main.cpp`
3. Build and upload

## Usage

### Automatic Operation

1. Connect the sniffer to the RS485 bus (A, B, GND)
2. Power on the ESP32
3. Open Serial Monitor at 115200 baud
4. The sniffer will:
   - Start with default 4800 baud
   - Auto-detect baud rate if data is present
   - Capture and display all RS485 traffic

### Manual Commands

Press a key in the Serial Monitor to execute commands:

- **d** - Detect baud rate manually
- **s** - Set specific baud rate (will prompt for value)
- **c** - Clear capture buffer
- **h** - Show help menu

### Output Format

```
========================================
Time: 15s | Baud: 4800
========================================
48 65 6C 6C 6F (H) 20 ( ) 57 6F 72 6C 64 (o) 0D (. ) 0A (. )
```

- **Hex values**: Raw byte values in hexadecimal
- **ASCII**: Printable characters shown in parentheses
- **(.)**: Non-printable characters shown as dots

## How Baud Rate Detection Works

The sniffer uses edge-change interrupts on the RX pin to measure pulse widths:

1. Captures timing of signal transitions
2. Finds the shortest reliable pulse (1 bit time)
3. Calculates estimated baud rate: `baud = 1,000,000 / pulse_width_us`
4. Matches to the nearest standard baud rate
5. Reinitializes UART with detected baud rate

This method works best with active bus traffic. If the bus is idle, use manual configuration.

## Troubleshooting

### No Data Received

- Verify wiring: A+ to A, B- to B, GND to GND
- Check that XY-485 VCC is connected to 3.3V
- Use 'd' command to detect baud rate
- Try manually setting baud rate with 's' command
- Verify RS485 bus has activity (use oscilloscope if available)

### Garbled Data

- Baud rate mismatch - use auto-detection or set manually
- Noise on bus - check termination resistors (120Ω across A-B)
- Ground loop - ensure proper grounding

### Auto-detection Fails

- Insufficient traffic on bus (need at least 10 edges)
- Non-standard baud rate - use manual setting
- Very slow data rate - increase detection timeout

## Advanced Configuration

### Changing Default Pins

Edit these lines in the code:
```cpp
#define RS485_RX_PIN 16      // GPIO16
#define RS485_TX_PIN 17      // GPIO17
```

### Adding More Baud Rates

Add to the `commonBaudRates` array:
```cpp
const unsigned long commonBaudRates[] = {1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400};
```

## Safety Notes

- RS485 bus voltage should be within ±7V (standard) or ±12V (extended)
- XY-485 modules are typically isolated but verify your specific module
- Do not connect to live mains or high voltage systems
- Use appropriate ESD protection when handling electronics

## License

MIT License - feel free to modify and distribute

## References

- [RS485 Standard](https://www.tiaonline.org/)
- [ESP32 Technical Reference](https://www.espressif.com/en/support/documents/technical-documents)
- XY-485 Module Datasheet (varies by manufacturer)