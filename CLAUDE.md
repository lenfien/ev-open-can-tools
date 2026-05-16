# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Memory System

**IMPORTANT — OVERRIDE DEFAULT MEMORY PATH**: All persistent memory files for this project are stored in `.claude/memory/` inside this repository (tracked by git), NOT in the system default path (`~/.claude/projects/.../memory/`). Always read from and write to `.claude/memory/` in the repo root. The index file is `.claude/memory/MEMORY.md`.

## Build System

This project uses **PlatformIO**. The user-editable `platformio_profile.h` must be configured before building.

```bash
# Configure profile (driver + vehicle + features)
python scripts/platformio_set_profile.py --driver DRIVER_TWAI --vehicle HW4 --enable EMERGENCY_VEHICLE_DETECTION

# Build for the current environment (see platformio.ini for defined envs)
pio run -e waveshare_ESP32_S3_RS485_CAN

# Flash and monitor
pio run -e waveshare_ESP32_S3_RS485_CAN --target upload
pio device monitor --baud 115200
```

**Offline UI preview** (no hardware needed — parses schema directly from handlers.cpp):
```bash
python3 scripts/build_preview.py
# Opens scratch/preview.html — all ESP32 HTTP calls are mocked in JS
```

## Linting

```bash
git ls-files '*.cpp' '*.h' '*.hpp' | xargs clang-format --dry-run --Werror --style=file
```

## Architecture

Logic is split between `include/` (types, interfaces, web UI) and `src/` (implementations). `src/main.cpp` is the thin PlatformIO/Arduino entry point that wires up the driver and calls Arduino `setup()` / `loop()`.

### Data Flow

1. CAN frame received via hardware driver (`include/drivers/`)
2. Arduino `loop()` calls `CanHandler::Handle()` with each frame
3. Handler reads/updates `CanState` and modifies the frame (bit manipulation, checksum)
4. Modified frame sent back on CAN bus only if `should_send && m_cnf.enable_inject`
5. Bit 52 of frame 1021/mux-0 is always forced to 0 in `loop()` before send (ban protection — never set this bit, it directly triggers a 1-week suspension)
6. `WebSetup()` registers HTTP routes; `mcpDashOnFrame()` is called per-frame for rate telemetry

### Key Files

| File | Role |
|------|------|
| `src/main.cpp` | Arduino `setup()` / `loop()` — driver init, main CAN read/send loop; defines `g_can_handler` / `g_can_driver` |
| `src/handlers.cpp` | `CanHandler::Handle()` implementation; `kCnfSchema[]` / `kStateSchema[]` definitions; `SaveConf()`, `LoadConf()` |
| `src/web_logic.cpp` | HTTP API handlers, WiFi management, OTA, settings persistence; NVS namespace `"ADunlock"` |
| `src/web_ui.cpp` | Contains the `DASH_HTML` raw string (embedded single-page app) |
| `include/common.h` | `CanFrame` — `id`, `dlc`, `data[8]`; `GetMux()`, `SetBit()` (bits are LSB-first: bit N = byte N/8, mask 1«(N%8)); `g_can_handler` / `g_can_driver` extern declarations |
| `include/handlers.h` | `CanConf`, `CanState`, `DebugState`, `CanHandler` structs; schema descriptor types (`CnfFieldDesc`, `StateFieldDesc`) |
| `include/web_logic.h` | HTTP route declarations, WiFi/AP state, web server instance (port 80) |
| `include/drivers/can_driver.h` | Abstract driver interface (init / read / send / setFilters) |
| `platformio_profile.h` | User config: driver selection, vehicle variant, GPIO pins, credentials, feature flags |
| `scripts/platformio_set_profile.py` | CLI to write `platformio_profile.h` |
| `scripts/build_preview.py` | Generates `scratch/preview.html` — offline UI preview with mocked ESP32 backend |
| `scripts/ev_can_analyzer.py` | Offline CAN log analysis tool |

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

### Schema-Driven UI (`kCnfSchema` / `kStateSchema`)

Both writable config fields (`CanConf`) and read-only state fields (`CanState`) are described by static arrays in `src/handlers.cpp`. The frontend fetches `/schema` once on load and auto-renders all controls and status tiles. CAN bit mappings are **not** in the schema — they remain hand-written in the per-ID handler functions.

- Add a writable toggle: add `uint32_t` field to `CanConf` + one `CNF_BOOL(...)` line in `kCnfSchema[]`.
- Add a read-only tile: add `uint32_t` field to `CanState` + one `STATE_NUM/STATE_ENUM(...)` line in `kStateSchema[]`.
- Fields with `hidden=true` (`CNF_HIDDEN_*` macros) are still serialised over `/config` and `/status` but the auto-renderer skips them — the frontend uses hand-written JS widgets for those.

### Debug Override (`DebugState`)

`CanHandler::m_dbg` holds per-frame bit masks that let the debug page override individual CAN bits at runtime without rebuilding firmware. Overrides are persisted to NVS key `"dbg_ovr"` and survive reboots. The four covered frames are `1016`, `1021/mux-0`, `1021/mux-1`, `1021/mux-2`.

### `others/mod_fsd.h`

Reference implementation of the FSD injection logic using an older `FSDConfig`-based API. Not included in any build target — kept as documentation of the CAN encoding (speed offset raw value, HW3 slew-rate limiter, etc.).

## Companion Router Project (`router/`)

`router/tesla-iso-1.0.3/` is a self-contained OpenWrt/BusyBox package that **network-isolates the Tesla in-car MCU** to block Tesla's remote FSD revocation channel (DNS sinkhole for `*.tesla.cn / *.tesla.com`, firewall, busybox httpd web console). It is installed on a separate travel router placed between the Tesla and the internet — unrelated to the ESP32 firmware but shipped in the same repo.

Install on the router:
```sh
sh /tmp/tesla-iso-1.0.3/install.sh
# Web console: http://<router-LAN-IP>:8888
```

## Plugin System

JSON-based CAN modification rules can be installed at runtime via the web dashboard (URL, file upload, or paste). The firmware detects conflicts between plugin rules and base firmware logic. Plugin state is separate from `CanConf` NVS storage.

## Versioning

- Project version lives in `VERSION` (Semantic Versioning).
- All changes go into the `Unreleased` section of `CHANGELOG.md` before merge.
- `scripts/check_release_metadata.py` enforces `VERSION` / `CHANGELOG.md` consistency at release time.

## CI / Release

GitHub Actions runs: clang-format lint → multi-board builds → release artifact upload. (`scripts/check_release_metadata.py` is run before tagging to enforce `VERSION` / `CHANGELOG.md` consistency.)
