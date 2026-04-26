# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build System

This project uses **PlatformIO**. The user-editable `platformio_profile.h` must be configured before building.

```bash
# Configure profile (driver + vehicle + features)
python scripts/platformio_set_profile.py --driver DRIVER_TWAI --vehicle HW4 --enable EMERGENCY_VEHICLE_DETECTION

# Build for specific environment
pio run -e esp32_twai
pio run -e feather_rp2040_can
pio run -e feather_m4_can
```

**Supported environments** (defined in `platformio.ini`):
`feather_rp2040_can`, `feather_m4_can`, `esp32_twai`, `esp32_feather_v2_mcp2515`, `lilygo_tcan485_hw3`, `m5stack-atomic-can-base`, `m5stack-atoms3-mini-can-base`, `esp32_ext_mcp2515`, `waveshare_ESP32_S3_RS485_CAN`

## Testing & Linting

```bash
# Native C++ tests (no hardware required)
pio test -e native
pio test -e native_bypass_tlssc_requirement
pio test -e native_log_buffer
pio test -e native_nag

# Run a single test suite
pio test -e native -f test_native_hw4

# Python tests
python test/test_can_analyzer.py

# Linting (clang-format)
git ls-files '*.cpp' '*.h' '*.hpp' | xargs clang-format --dry-run --Werror --style=file
```

## Architecture

Logic is split between `include/` (types, interfaces, web UI) and `src/` (implementations). `src/main.cpp` is the thin PlatformIO entry point that wires up the driver and calls `AppSetup()` / `AppLoop()`.

### Data Flow

1. CAN frame received via hardware driver (`include/drivers/`)
2. `AppLoop()` calls `CanHandler::Handle()` with each frame
3. Handler reads/updates `CanState` and modifies the frame (bit manipulation, checksum)
4. Modified frame sent back on CAN bus only if `should_send && m_cnf.enable_inject`
5. Bit 52 of frame 1021/mux-0 is always forced to 0 before send (ban protection)
6. Web dashboard (`WebSetup` / `mcpDashboardLoop`) runs on ESP32 for config and telemetry

### Key Files

| File | Role |
|------|------|
| `src/app.cpp` | `AppSetup()` / `AppLoop()` — driver init, main CAN read/send loop |
| `src/handlers.cpp` | `CanHandler::Handle()` implementation; `PrintCnf()`, `PrintState()`, `SaveConf()`, `LoadConf()` |
| `src/web_logic.cpp` | HTTP API handlers, WiFi management, OTA, settings persistence |
| `include/app.h` | Global `g_can_handler` / `g_can_driver` declarations; `AppSetup()` / `AppLoop()` prototypes |
| `include/handlers.h` | `CanConf`, `CanState`, `CanHandler` structs; `m_gtw_protector` ban-shield frame table |
| `include/can_frame_types.h` | `CanFrame` — `id`, `dlc`, `data[8]`; `GetMux()`, `SetBit()` helpers |
| `include/web_logic.h` | HTTP route declarations, WiFi/AP state, web server instance |
| `include/web_ui.h` | Embedded single-page app HTML/JS (CAN sniffer, feature toggles, WiFi config, OTA) |
| `include/drivers/can_driver.h` | Abstract driver interface (init / read / send / setFilters) |
| `include/drivers/mock_driver.h` | Mock driver used in native tests |
| `platformio_profile.h` | User config: driver selection, vehicle variant, GPIO pins, credentials, feature flags |
| `scripts/platformio_set_profile.py` | CLI to write `platformio_profile.h` |

### Runtime Configuration — `CanConf` and `CanState`

`CanHandler` holds two key structs:

- **`CanConf m_cnf`** — persistent configuration, saved/loaded via `SaveConf()`/`LoadConf()` to NVS key `"can_cnf"`. Contains all feature toggles (`enable_fsd`, `enable_inject`, `enable_ban_shield`, etc.), speed profile settings, and the 12-entry `speed_limit_auto_cfg[]` table. Version-checked on load; mismatched versions discard stored config.
- **`CanState m_state`** — ephemeral runtime state (frame counters, current follow distance, speed profiles in use, ban-shield counters, gateway autopilot value).

`PrintCnf()` and `PrintState()` dump all fields via `Serial.printf` — gated on `m_cnf.enable_print`.

### CAN ID Responsibilities (frame 1021)

Frame 1021 is muxed (lower 3 bits of `data[0]`):
- **mux 0** — FSD enable bits 46/60; emergency vehicle detection bit 59; HW3 speed profile in `data[6][2:1]`
- **mux 1** — nag suppression bits 19/47; camera disable bit 43
- **mux 2** — HW4 speed profile in `data[7][6:4]`; speed offset byte

### Ban Shield (`m_gtw_protector`)

Frame 2047 contains a 10-mux sequence. The shield stores the last-seen frame per mux; only forwards a mux frame when its content has changed. `ban_shield_cnt` / `ban_shield_check_cnt` track hit rate.

### `others/mod_fsd.h`

Reference implementation of the FSD injection logic using an older `FSDConfig`-based API. Not included in any build target — kept as documentation of the CAN encoding (speed offset raw value, HW3 slew-rate limiter, etc.).

## CI / Release

GitHub Actions (`.github/workflows/ci.yml`) runs: clang-format lint → native tests → multi-board builds → release artifact upload. `scripts/check_release_metadata.py` validates `VERSION` and `CHANGELOG.md` consistency before release.
