# AxeForge Roadmap

Ideas and enhancements beyond the current feature set:

5. **Day/Night & Time-of-Day Scheduling**
   - Local time-of-day profiles (e.g., quiet/low-power mode overnight with capped fan or power limit, full performance during the day) alongside existing electricity-price cutoffs.

8. **Swarm Bulk OTA & Settings Sync**
   - Push a firmware `.bin` or sync pool/theme/display settings across all AxeForge miners on the Swarm tab in one click.

9. **Configurable Pool-Side Minimum Difficulty (`SUGGEST_DIFF`)**
   - Expose `mining.suggest_difficulty` per pool so users can tune share frequency directly (e.g., higher difficulty to quiet BitHalo LED pulses or lower difficulty for smoother short-window hashrate).

10. **Latency & Stale-Share Aware Failover**
    - Track pool submit latency (`ping_ms`) and consecutive rejected/stale shares to temporarily demote degraded pools in Failover mode.

12. **Multi-Point Fan Curve**
    - Complement the single PI target (`fantgt`) with a 2–3 point fan curve (quiet floor + ramp slope) so desktop miners stay silent at low ambient temps and ramp smoothly before the 3 °C ceiling boost.

13. **Stale vs. Hardware-Error Breakdown**
    - Track and display stale/duplicate share counts separately from ASIC hardware errors on *Pools* and *Dashboard* to distinguish network/pool lag from unstable overclocks.

14. **Build Commit / Timestamp on *System → Device***
    - Expose the short git SHA and build timestamp alongside the firmware version so custom/forked builds are immediately distinguishable in the web UI and Swarm table.

15. **Configurable Log / BitHalo Quiet Mode**
    - Allow toggling or rate-limiting per-share `share accepted` logging on UART0 for users on ultra-low-difficulty pools where the BitHalo pulses too frequently.

16. **24h Telemetry CSV / JSON Export**
    - Allow downloading the 24-hour hashrate, chip temperature, and power history buffer as CSV from *Dashboard* for offline graphing or comparing cooling mods.

17. **Swarm-Aggregated Prometheus Metrics (`/metrics`)**
    - Extend `/metrics` with an optional `?swarm=1` query so a single Prometheus/Grafana scrape target can collect metrics for the entire miner fleet.

18. **Auto-Tune Profile Presets (Quiet / Balanced / Max OC)**
    - One-click presets in *Tuning* that configure power cap, fan target, and goal (`efficiency` vs `max hashrate`) together.

19. **Per-Chip Degradation Alerts (Discord / Telegram)**
    - Send a warning notification when a single chip's error rate exceeds a user-defined threshold or its hashrate lags >15% behind siblings, catching drying thermal paste before an emergency stall cut.

20. **Auto Day/Night Web & Screen Theme**
    - Automatically switch between *Paper* (light) during daytime hours and *Forge* / *Ember* / *DTV* (dark) at night using the NTP-synced local clock.
