# DDS Signal Generator — ESP32 + AD9833 with Web UI

![build](https://github.com/che711/signal-generator-esp32-ad9833/actions/workflows/build.yml/badge.svg)

A compact, standalone function generator built with an ESP32 microcontroller
and the AD9833 DDS module. Controlled via a rotary encoder or a built-in
web interface, with status shown on a 0.96" SSD1306 OLED screen.
Settings (frequency, waveform, step) are persisted to flash (NVS) and
restored on boot.

---

## Features

| Feature | Details |
|---------|---------|
| Frequency range | **0.1 Hz – 12 MHz** |
| Frequency resolution | **0.1 Hz** |
| Waveforms | Sine, Triangle, Square, Square/2 |
| Frequency steps | 0.1 Hz / 1 Hz / 10 Hz / 100 Hz / 1 kHz / 10 kHz / 100 kHz / 1 MHz |
| Acceleration | Fast encoder spin = 10× bigger step |
| Web interface | Embedded single-page UI: set frequency/waveform/step, sweep control, curl cheatsheet, live system stats |
| Sweep | Linear / logarithmic frequency sweep, 0.2 s – 1 h, phase-continuous |
| Output enable | Mute button (web/API): DDS sleep, settings kept, state survives reboot |
| mDNS | `http://dds-gen.local` (no need to know the IP) |
| Persistence | Auto-save to NVS 5 s after the last change + manual save from web UI |
| WiFi watchdog | Automatic reconnect if the connection drops |
| Display | SSD1306 128×64 OLED (I2C) |
| Control | KY-040 rotary encoder (interrupt-driven quadrature decoder) |
| Output | BNC connector (50 Ω) |
| Power | 5V SMPS + LC filter |

---

## Controls

### Rotary encoder

| Action | Result |
|--------|--------|
| Rotate encoder | Increase / decrease frequency by current step |
| Fast rotation | Step × 10 acceleration |
| Short press | Cycle waveform (SINE → TRI → SQR → SQR/2) |
| Long press (>0.8s) | Cycle frequency step decade |

### Web interface

Open `http://dds-gen.local` (or the IP shown on the OLED at boot).
The page lets you type an exact frequency, pick the waveform and step,
nudge the frequency by the current step, save settings to memory, and
shows live system stats (RSSI, CPU load, RAM, chip temperature, uptime).

HTTP API used by the page (usable from scripts too):

| Endpoint | Effect |
| -------- | ------ |
| `GET /status` | JSON: frequency, waveform, step + system stats |
| `GET /set/freq?v=<hz>` | Set frequency (0.1 – 12 000 000 Hz) |
| `GET /set/wave?v=<name>` | Waveform: `sine`, `tri`, `sqr`, `sqr2` (legacy `0`-`3` accepted) |
| `GET /set/step?v=<size>` | Encoder step, Hz added per click: `0.1`,`1`,`10`,`100`,`1k`,`10k`,`100k`,`1m` (legacy `0`-`7`) |
| `GET /set/out?v=<0\|1>` | Output enable: `0` mutes the DDS (sleep, ~0 V out), `1` restores the selected waveform |
| `GET /save` | Persist current settings to NVS |
| `GET /reboot` | Save settings, then restart the device (`ESP.restart()`) |
| `GET /sweep/start?f0=&f1=&t=&mode=` | Frequency sweep f0 → f1 Hz over `t` seconds, `mode` = `lin` \| `log` |
| `GET /sweep/stop` | Stop sweep mid-run: frequency stays at its current value. A sweep that finishes on its own returns to the pre-sweep frequency |

Every `/set/*` and `/sweep/*` endpoint replies with the same JSON as `/status`,
reflecting the state actually applied (values are clamped to valid ranges).

<details>
<summary><b>curl examples</b> — scripts, Robot Framework, lab automation</summary>

```bash
# Device state: frequency, waveform, sweep, RSSI, heap, uptime
curl http://dds-gen.local/status

# 10 kHz sine
curl "http://dds-gen.local/set/freq?v=10000"
curl "http://dds-gen.local/set/wave?v=sine"   # sine | tri | sqr | sqr2

# Log sweep 10 Hz -> 100 kHz over 10 s (Bode plot on the scope)
curl "http://dds-gen.local/sweep/start?f0=10&f1=100000&t=10&mode=log"
curl http://dds-gen.local/sweep/stop

# Persist settings to NVS
curl http://dds-gen.local/save
```

Robot Framework setup example:

```robotframework
*** Settings ***
Library    RequestsLibrary

*** Keywords ***
Set Reference Signal
    [Arguments]    ${freq}    ${wave}=0
    GET    http://dds-gen.local/set/freq    params=v=${freq}
    GET    http://dds-gen.local/set/wave    params=v=${wave}
```
</details>

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

See [wiring.md](wiring.md) for full pin mapping and ASCII schematic.

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

All pins are defined in [src/config.h](src/config.h).

---

## Configuration

Before flashing, edit [src/config.h](src/config.h):

```c
#define WIFI_SSID      "YourNetwork"
#define WIFI_PASSWORD  "YourPassword"
#define MDNS_HOSTNAME  "dds-gen"      // → http://dds-gen.local
```

If WiFi is unavailable the generator still works standalone with the
encoder and OLED.

> Don't commit real WiFi credentials to a public repository.

---

## Build & Flash

### PlatformIO (recommended)

The project uses the [pioarduino](https://github.com/pioarduino/platform-espressif32)
platform (Arduino core 3.x / ESP-IDF 5.x); dependencies are resolved
automatically from `platformio.ini`.

```bash
git clone https://github.com/che711/signal-generator-esp32-ad9833
cd signal-generator-esp32-ad9833
pio run --target upload
pio device monitor
```

### Arduino IDE

1. Install ESP32 board support (arduino-esp32 core 3.x)
2. Install libraries:
   - `AD9833` by Rob Tillaart
   - `U8g2` by olikraus
3. Copy the contents of `src/` into a sketch (rename `main.cpp` to the sketch `.ino`)
4. Select board: **ESP32 Dev Module**
5. Upload

---

## Project Structure

```
signal-generator-esp32-ad9833/
├── src/
│   ├── main.cpp          # Setup & main loop, autosave logic
│   ├── config.h          # Pins, WiFi credentials, timings
│   ├── generator.h/.cpp  # AD9833 wrapper (frequency, waveform, step, NVS)
│   ├── encoder.h/.cpp    # KY-040 encoder (ISR quadrature decoder, debounced button)
│   ├── display.h/.cpp    # SSD1306 OLED screens
│   └── webui.h/.cpp      # WiFi, web server, mDNS, embedded HTML UI
├── wiring.md             # Full wiring guide + ASCII schematic
├── platformio.ini
└── README.md
```

---

## Display Layout

```
┌────────────────────────┐
│ WAVE: SINE         [W] │  ← inverted top bar, [W] = WiFi connected
├────────────────────────┤
│      1.000 kHz         │  ← large frequency (centred)
├────────────────────────┤
│ STP: 1kHz              │
│ NET: MyNetwork         │  ← SSID
│ IP:  192.168.1.45      │  ← web UI address
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
[DDS] Booting v3...
[GEN] Settings loaded from NVS
[GEN] freq=1000.00Hz wave=0 step=4
[WiFi] Connecting to SkyNet....
[WiFi] IP: 192.168.1.45
[mDNS] http://dds-gen.local
[Web] http://192.168.1.45
[DDS] Ready
[ENC] CW → 2000.00 Hz
[BTN] wave → TRI
[Web] freq → 5000.00 Hz
[GEN] Settings saved to NVS
```

---

## License

MIT License — free for personal and commercial use.
