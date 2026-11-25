# AxeForge Roadmap

Ideas and enhancements beyond the current feature set:

2. **Scoped Read-Only / Automation API Token**
   - Allow a configurable Bearer token for `/api/metrics`, `/api/status`, and `/api/swarm` so Prometheus, Home Assistant, and Swarm polling keep working when 2FA (`otp::enabled()`) is enabled.

3. **Proactive Thermal-Aware Per-Chip Clock Balancing**
   - Balance per-chip clock offsets in `tuner.cpp` against each chip's die temperature headroom (`tmp451::chip(i)` / `chip_temp_own[i]`) before hitting a chain-wide thermal wall.

4. **Dynamic ASIC Difficulty Auto-Scaling & `mining.extranonce.subscribe`**
   - Target a steady per-chip nonce rate (~1–2 nonces/s) via `mining.suggest_difficulty`, and support `mining.extranonce.subscribe` in Stratum V1 plus `POST /api/pools/test` before saving.

5. **Structured PMBus Fault Black-Box & Fan Bearing Baseline**
   - Save a structured pre-shutdown snapshot (PMBus `STATUS_WORD` + sub-registers, `vin`/`vout`/`iout`, chip temps, PSU sag) on emergency cuts, and track fan RPM-vs-duty baseline to catch bearing wear early and persist the learned `fan_floor[i]`.

6. **Night-Clock Dim Mode & Screen Preview Compression**
   - Add an optional dim night-clock brightness (e.g. 10%) instead of 0% backlight off, plus RLE/dirty-frame `/api/screen.bmp` and a Swarm quick-switcher in the web header.
