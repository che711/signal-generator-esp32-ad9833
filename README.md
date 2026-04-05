# DDS Signal Generator — ESP32 + AD9833

A compact, standalone function generator built with an ESP32 microcontroller
and the AD9833 DDS module, controlled via a rotary encoder and displayed on a
0.96" SSD1306 OLED screen.

---

## Features

| Feature | Details |
|---------|---------|
| Frequency range | **0.1 Hz – 12 MHz** |
| Frequency resolution | **0.1 Hz** |
| Waveforms | Sine, Triangle, Square, Square/2 |
| Frequency steps | 0.1 Hz / 1 Hz / 10 Hz / 100 Hz / 1 kHz / 10 kHz / 100 kHz / 1 MHz |
| Acceleration | Fast encoder spin = 10× bigger step |
| Display | SSD1306 128×64 OLED (I2C) |
| Control | KY-040 rotary encoder |
| Output | BNC connector (50 Ω) |
| Power | 5V SMPS + LC filter |

---

## Controls

| Action | Result |
|--------|--------|
| Rotate encoder | Increase / decrease frequency by current step |
| Fast rotation | Step × 10 acceleration |
| Short press | Cycle waveform (SINE → TRI → SQR → SQR/2) |
| Long press (>0.8s) | Cycle frequency step decade |

---

## Hardware

| Component | Description |
|-----------|-------------|
| ESP32 DevKit / Super Mini | Main controller |
| AD9833 module | DDS signal generator (SPI) |
| SSD1306 0.96" | OLED display (I2C) |
| KY-040 | Rotary encoder with push button |
| 5V 1A SMPS | Power supply (barrel jack 5.5×2.1 mm) |
| LC filter | Suppresses switching noise from SMPS |
| BNC socket (isolated) | Output connector |
| 100 Ω resistor | Series protection on BNC output |
| RG58 coax cable + BNC plugs | Output cable to oscilloscope |

---

## Wiring

See [docs/wiring.md](docs/wiring.md) for full pin mapping and ASCII schematic.

### Quick reference (ESP32 DevKit)

```
AD9833  CLK  → GPIO 18
AD9833  DAT  → GPIO 23
AD9833  FSYNC→ GPIO  5

OLED    SDA  → GPIO 21
OLED    SCL  → GPIO 22

KY-040  CLK  → GPIO 34
KY-040  DT   → GPIO 35
KY-040  SW   → GPIO 32

AD9833 OUT → [100Ω] → BNC centre
AD9833 GND          → BNC shell
```

---

## Build & Flash

### PlatformIO (recommended)

```bash
git clone https://github.com/YOUR_USERNAME/signal-generator-esp32-ad9833
cd signal-generator-esp32-ad9833
pio run --target upload
pio device monitor
```

### Arduino IDE

1. Install libraries:
   - `AD9833` by Rob Tillaart
   - `U8g2` by olikraus
2. Open `src/main.cpp`
3. Select board: **ESP32 Dev Module**
4. Upload

---

## Project Structure

```
signal-generator-esp32-ad9833/
├── src/
│   ├── main.cpp          # Setup & main loop
│   ├── generator.h/.cpp  # AD9833 wrapper (frequency, waveform, step)
│   ├── encoder.h/.cpp    # KY-040 encoder (rotation, click, long-click)
│   └── display.h/.cpp    # SSD1306 OLED screens
├── docs/
│   └── wiring.md         # Full wiring guide + ASCII schematic
├── platformio.ini
└── README.md
```

---

## Display Layout

```
┌────────────────────────┐
│ WAVE: SINE             │  ← inverted top bar
├────────────────────────┤
│                        │
│      1.000 kHz         │  ← large frequency (centred)
│                        │
├────────────────────────┤
│ STEP: 1kHz             │  ← bottom bar
└────────────────────────┘
```

---

## Output Notes

- AD9833 output amplitude: ~0.65 Vpp (sine/triangle)
- For higher amplitude add an op-amp stage (e.g. AD8051, TL071)
- Square wave output: up to ~3V logic level
- Add a 100 Ω series resistor on the BNC output to prevent damage on short circuit
- Use 50 Ω coax (RG58) for best signal integrity above 1 MHz

---

## Serial Monitor

At 115200 baud the device prints events:

```
[DDS Generator] booting...
[DDS Generator] ready
[ENC] CW  → 2000.00 Hz
[ENC] CW  → 3000.00 Hz
[BTN] wave → TRI
[BTN] step → 10kHz
```

---

## License

MIT License — free for personal and commercial use.
