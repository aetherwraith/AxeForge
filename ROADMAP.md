# AxeForge Roadmap

Ideas and enhancements beyond the current feature set:

17. **Swarm-Aggregated Prometheus Metrics (`/metrics`)**
    - Extend `/metrics` with an optional `?swarm=1` query so a single Prometheus/Grafana scrape target can collect metrics for the entire miner fleet.

18. **Auto-Tune Profile Presets (Quiet / Balanced / Max OC)**
    - One-click presets in *Tuning* that configure power cap, fan target, and goal (`efficiency` vs `max hashrate`) together.

19. **Per-Chip Degradation Alerts (Discord / Telegram)**
    - Send a warning notification when a single chip's error rate exceeds a user-defined threshold or its hashrate lags >15% behind siblings, catching drying thermal paste before an emergency stall cut.

20. **Auto Day/Night Web & Screen Theme**
    - Automatically switch between *Paper* (light) during daytime hours and *Forge* / *Ember* / *DTV* (dark) at night using the NTP-synced local clock.
