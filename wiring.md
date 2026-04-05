# Wiring Guide

## Power

```
230V AC → [5V 1A SMPS] → [LC Filter] → 5V rail
                                          │
                              ┌───────────┼──────────────┐
                           ESP32        AD9833        SSD1306
                          (5V/VIN)    (3.3V via      (3.3V via
                                       ESP32)         ESP32)
```

## ESP32 → AD9833 (SPI / HSPI)

| ESP32 Pin | AD9833 Pin | Description       |
|-----------|-----------|-------------------|
| GPIO 18   | CLK       | SPI clock         |
| GPIO 23   | DAT       | SPI data (MOSI)   |
| GPIO  5   | FSYNC     | Chip select (CS)  |
| 3.3V      | VCC       | Power             |
| GND       | GND       | Ground            |

> **Note:** AD9833 MISO not used. FSYNC is active-LOW chip select.

## ESP32 → SSD1306 OLED (I2C)

| ESP32 Pin | SSD1306 Pin | Description |
|-----------|------------|-------------|
| GPIO 21   | SDA        | I2C data    |
| GPIO 22   | SCL        | I2C clock   |
| 3.3V      | VCC        | Power       |
| GND       | GND        | Ground      |

> Default I2C address: `0x3C`. Some modules use `0x3D`.
> Check with I2C scanner if display is blank.

## ESP32 → KY-040 Encoder

| ESP32 Pin | KY-040 Pin | Description              |
|-----------|-----------|--------------------------|
| GPIO 34   | CLK       | Rotation clock (input-only) |
| GPIO 35   | DT        | Rotation data  (input-only) |
| GPIO 32   | SW        | Button                   |
| 3.3V      | +         | Power                    |
| GND       | GND       | Ground                   |

> GPIO 34 and 35 are **input-only** on ESP32 — no internal pull-up.
> KY-040 has its own pull-up resistors on board, so this is fine.

## BNC Output

| AD9833 Pin | BNC Jack |
|-----------|----------|
| OUT       | Centre pin (signal) |
| GND       | Shell (ground)      |

> Add a **100 Ω series resistor** between AD9833 OUT and BNC centre
> to protect against short circuits.

## Full Schematic (ASCII)

```
                    ┌──────────────────────────────────────┐
   5V ──────────────┤VIN                              GPIO18├──── AD9833 CLK
   GND ─────────────┤GND                              GPIO23├──── AD9833 DAT
                    │                                 GPIO5 ├──── AD9833 FSYNC
                    │                                       │
                    │         ESP32 DevKit            GPIO21├──── OLED SDA
                    │                                 GPIO22├──── OLED SCL
                    │                                       │
                    │                                 GPIO34├──── KY-040 CLK
                    │                                 GPIO35├──── KY-040 DT
                    │                                 GPIO32├──── KY-040 SW
                    │                                       │
                    │   3.3V ─┬──────────────────────       │
                    │         ├── AD9833 VCC                 │
                    │         └── OLED VCC                  │
                    └──────────────────────────────────────┘

AD9833 OUT ──[100Ω]──── BNC centre pin
AD9833 GND ─────────── BNC shell
```

## ESP32 Super Mini Pin Mapping

If using **ESP32 Super Mini** (C3), note it has **no hardware DAC**.
The AD9833 SPI still works — update pin numbers in `generator.h`:

| Function | DevKit | Super Mini |
|----------|--------|-----------|
| SPI SCK  | 18     | 4         |
| SPI MOSI | 23     | 6         |
| SPI CS   | 5      | 7         |
| I2C SDA  | 21     | 8         |
| I2C SCL  | 22     | 9         |
| ENC CLK  | 34     | 2         |
| ENC DT   | 35     | 3         |
| ENC SW   | 32     | 10        |
