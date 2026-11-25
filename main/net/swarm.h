#pragma once
#include <stddef.h>
#include <stdint.h>

// Swarm: the other miners on the LAN, polled by this miner so the browser
// never has to talk to them (keeps the API guard strict, no CORS needed).
// Understands the AxeOS / ESP-Miner-NerdQAxePlus API (/api/system/info) and
// AxeForge's own (/api/status). The list is kept in NVS.
namespace swarm {
void start();
char *to_json();               // caller frees
bool add(const char *ip);      // false if not a valid IPv4 address or the list is full
bool remove(const char *ip);
void request_scan();           // probes the local /24 for miners, in the background
void request_sync(uint8_t mask); // 1=pools, 2=display/theme/clock, 3=both
void request_bulk_update();    // triggers GitHub release OTA on online AxeForge peers
bool start_bulk_ota(char *psram_bin, size_t len); // takes ownership of psram_bin
const char *bulk_status();
size_t metrics_prom(char *out, size_t cap);
void flush();                  // the monitor task: writes a changed list to flash
}
