# RS485 Sniffer

**Main Utility: `rs485_sniffer/rs485_sniffer.ino`**

An Arduino-based RS485 communication monitor for ESPHome devices. This utility captures and displays RS485 data frames in real-time, with support for Modbus protocol decoding.

## Quick Start

The main Arduino sketch is located at:
```
rs485_sniffer/rs485_sniffer.ino
```

### Requirements

- Arduino/ESP board with hardware serial support
- MAX485 RS485 module
- ESPHome-compatible RS485 device to monitor

### Wiring

```
MAX485 Module    →  ESP Board
────────────────────────────
RO               →  GPIO 16 (RX)
RE               →  GPIO 19
DE               →  GPIO 18
```

## Features

- **Real-time monitoring**: Captures RS485 data frames as they're transmitted
- **Two display modes**:
  - Frame mode: Shows complete frames with timestamps
  - Stream mode: Single-line continuous output
- **Modbus decoding**: Automatically identifies and decodes Modbus function codes
- **ASCII display**: Shows printable ASCII characters
- **Statistics tracking**: Monitor uptime, frame count, byte count, and average frame size

## Configuration

- **Baud Rate**: 4800
- **Parity**: ODD
- **Data Bits**: 8
- **Stop Bits**: 1
- **Configuration**: 8-O-1

## Commands

- `s` - Toggle stream mode (single-line continuous output)
- `i` - Show statistics (uptime, frames, bytes, average)
- `c` - Clear counters (reset frame and byte counts)
- `h` - Show help

## Output Example

### Frame Mode Output
```
[12.345] Frame #1 - 6 bytes: 01 03 00 00 00 01       | Modbus: ID=1 FC=0x03 (Read Holding)
[12.352] Frame #2 - 7 bytes: 01 03 02 12 34 C8 1A    | Modbus: ID=1 FC=0x03 (Read Holding)
```

### Stream Mode Output
```
[Stream] 01 03 00 00 00 01 01 03 02 12 34 C8 1A
```

## Usage

1. Open `rs485_sniffer/rs485_sniffer.ino` in Arduino IDE
2. Upload to your ESP board
3. Open Serial Monitor at 115200 baud
4. Connect your RS485 device to monitor
5. Use commands to interact with the monitor

## Troubleshooting

- No data detected: Check RS485 wiring and connections
- Garbled data: Verify baud rate matches your device (default: 4800)
- Frames not decoding correctly: Ensure parity settings match (default: ODD)
