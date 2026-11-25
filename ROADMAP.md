# AxeForge Roadmap

Ideas and enhancements beyond the current feature set:

1. **Per-Chip V/F Curve Sweep & Persistent Pareto Profile**
   - One-click automated voltage/frequency sweep mapping each chip's error rate, J/TH, and thermal delta into a persistent lookup table for instant optimal power-cap and schedule jumps.

2. **Fleet-Wide Coordinated Power & Thermal Budgeting**
   - Share a total watt cap across AxeForge swarm peers (for a shared PSU/breaker/solar budget), throttling the least-efficient unit first when total draw or ambient temperature spikes.

3. **Local LAN Stratum Multiplexer / Mini-Proxy**
   - Allow one AxeForge node to hold a single upstream Stratum V1-TLS or V2 connection and serve deduplicated work to other LAN miners.

4. **Mempool Fee-Alert & Tracked-Pool Block Notifications**
   - Discord/Telegram alerts when recommended mempool fees drop below a user threshold or when a tracked pool mines a block.

5. **Fleet-Wide Swarm Actions**
   - Broadcast profile presets, `identify` blink, or staged firmware updates across selected AxeForge peers from one device.
