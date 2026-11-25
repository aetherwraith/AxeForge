# AxeForge Roadmap

Ideas and enhancements beyond the current feature set:

5. **Structured PMBus Fault Black-Box & Fan Bearing Baseline**
   - Save a structured pre-shutdown snapshot (PMBus `STATUS_WORD` + sub-registers, `vin`/`vout`/`iout`, chip temps, PSU sag) on emergency cuts, and track fan RPM-vs-duty baseline to catch bearing wear early and persist the learned `fan_floor[i]`.

6. **Night-Clock Dim Mode & Screen Preview Compression**
   - Add an optional dim night-clock brightness (e.g. 10%) instead of 0% backlight off, plus RLE/dirty-frame `/api/screen.bmp` and a Swarm quick-switcher in the web header.
