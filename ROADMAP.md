# AxeForge Roadmap

Ideas and enhancements beyond the current feature set:

1. **Per-Chip Thermal Derating (Die-Temp Guardrail)**
   - On multi-chip boards with per-chip die temperature sensors (NerdQX, NerdOctaxe-γ), proactively step down only the hottest chip when a single die approaches `AF_MAX_CHIP_TEMP`, preserving chain-wide hashrate instead of throttling all chips together.

2. **Silicon Quality / Binning Fingerprint ("Chip Score")**
   - Record each chip's measured `MHz / mV / W / err%` summary during auto-tune (climb, trim, and per-chip clock steps) and surface a per-chip silicon score and efficiency summary on the *ASIC chips* web tab to help spot weak paste/cooling or compare boards.

3. **Adaptive Job-Interval / Extranonce2 Rollover Tuning**
   - Dynamically scale the job interval based on live chip hashrate and 32-bit nonce space exhaustion time so high-overclock single-chip and multi-chip boards never wrap nonce space before the next job arrives, without wasting UART bandwidth at lower clocks.

4. **MQTT & Home Assistant Auto-Discovery**
   - Lightweight MQTT telemetry publisher with Home Assistant discovery topics, plus optional control topics (`pause` / `resume` / `power_limit`) for solar-surplus or external thermal/smart-plug automation.

5. **Day/Night & Time-of-Day Scheduling**
   - Local time-of-day profiles (e.g., quiet/low-power mode overnight with capped fan or power limit, full performance during the day) alongside existing electricity-price cutoffs.

6. **Hardware Watchdog & Brownout Telemetry Counters**
   - Persist `esp_reset_reason()` counts (brownout, task WDT, panic, power-on) in NVS and display them on *System → Device* to make diagnosing flaky 5 V cables, barrel jacks, or PSUs easier.
