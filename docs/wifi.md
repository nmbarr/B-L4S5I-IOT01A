# WiFi sensor access

Goal: view live sensor data (accel/gyro, temp/humidity, DS18B20 temp) from a
phone instead of only via UART/teleplot over a wired debug connection.

Onboard WiFi module: Inventek ISM43362-M3G-L44 (SPI + AT-command protocol).
Driver/protocol layer will come from ST's X-CUBE-WIFI1 middleware, vendored
into this repo the same way CMSIS/HAL are today, with a thin per-board SPI
port (CS/reset/data-ready IRQ wiring) written here.

## Why not in libdrivers

Considered adding a WiFi driver to `libdrivers` alongside the sensor drivers.
Decided against it: everything in `libdrivers` today is a small,
vendor-agnostic register-bus driver (handle + a few registers + read/write),
testable off-target with no HAL dependency. The ISM43362 is a networking
co-processor with connection state and AT-command framing — a different
shape of problem that doesn't reuse much of what's already there. Using ST's
own middleware directly avoids reinventing a WiFi stack.

## v1 — local, single device

Scope: the board joins a local WiFi network and serves current sensor
readings to one phone on the same network. No internet routing involved.

Decisions:
- **Transport**: minimal on-device HTTP server. Phone hits a URL in a
  regular browser (or `curl`), board responds with current readings as JSON.
  ```
  GET http://192.168.1.42/sensors

  {"temp_F":72.1,"humidity":41.2,...}
  ```
- **Push vs. poll**: phone polls. Board has no need to track connected
  clients or push timing; fits naturally with the HTTP request/response
  model above.
- **Discovery**: hardcoded/static IP for v1 — configure a static IP (or a
  DHCP reservation) and read/type it in. No mDNS responder needed yet.
- **Credentials**: SSID/password hardcoded at build time, same pattern as
  the I2C addresses and sensor configs already hardcoded in `main.c`.

Explicit non-goals for v1: authentication, multiple simultaneous clients,
access from outside the local network.

Open implementation questions (need deciding before/while implementing):
- **Credentials in git**: hardcoding SSID/password at build time is fine,
  but not directly in a tracked file — `main.c` is committed to a public
  repo. Use a gitignored header (e.g. `wifi_credentials.h`, included by
  `main.c`, with a checked-in `.example` template) rather than a literal
  `#define` in tracked source.
- **Where does the current reading live for the HTTP handler to read?**
  `temp_F`, `gyro_x`, etc. are all local variables inside a single
  `while(1)` iteration in `main.c` today — nothing persists them anywhere an
  HTTP request handler could reach. Need some shared "latest reading" state.
- **Blocking vs. non-blocking connection handling**: servicing an incoming
  HTTP request is a "wait for something to happen" operation. Deciding
  whether that's polled non-blockingly each loop iteration (same pattern the
  DS18B20 conversion state machine already uses) or handled some other way,
  rather than discovering it mid-implementation.

## v2 — remote, anyone from anywhere

Scope: any client, not just one on the same local network, can check sensor
data from anywhere on the internet.

This is a substantially bigger jump than v1, not just "open a port":
- The board can't safely be directly internet-exposed (no port-forwarding a
  bare embedded HTTP server onto the open internet) — likely needs the board
  to phone out to a relay/broker/backend service instead of accepting
  inbound connections directly.
- Needs some form of auth (this is no longer "if you're on my WiFi you're
  trusted").
- Needs a backend/service component this repo doesn't currently have any of
  (hosting, protocol choice — MQTT to a broker is a common fit for this kind
  of telemetry, but undecided).

Not started — v1's on-device data format/API should inform some of these
decisions once it exists.
