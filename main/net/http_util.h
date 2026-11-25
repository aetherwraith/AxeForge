#pragma once
#include <stddef.h>

namespace http {
// GET over HTTP(S) with the built-in CA bundle. Returns a NUL-terminated heap
// buffer (caller frees) or NULL on failure/non-200. `quiet` skips the warning
// log, for callers that expect misses (swarm polling of offline miners).
char *get(const char *url, size_t max_len, int timeout_ms = 8000, bool quiet = false);
// POST with X-AxeForge: 1 header. Returns HTTP status code or -1 on error.
int post(const char *url, const char *body, size_t len, const char *ctype = "application/json",
         int timeout_ms = 8000, bool quiet = false);
}
