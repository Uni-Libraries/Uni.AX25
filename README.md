# uni_ax25

AX.25 header (address field, control, PID) encoding and decoding in plain C.

- `uni_ax25_decode()` accepts 2 to 10 addresses where only the last one carries the extension bit,
  followed by one control byte and one PID byte (UI and modulo-8 I frames). It clears the output
  header first and returns the header size, or `SIZE_MAX` for a malformed or truncated header.
  Callsigns are NUL-terminated with the space padding removed.
- `uni_ax25_encode()` / `uni_ax25_encode_address()` write destination, source and the repeaters up to
  the first empty callsign, set the extension bit on the last address only, and return `SIZE_MAX`
  instead of a truncated address field when the buffer is too small.
- `uni_ax25_contains_callsign()` compares NUL-terminated callsigns (SSID ignored).

S and U frames (no PID) and modulo-128 control fields are not modelled. The FCS and HDLC bit stuffing
are left to the caller.

## Build

```bash
cmake -S . -B build -DCPM_USE_LOCAL_PACKAGES=ON
cmake --build build
ctest --test-dir build
```

Tests use an installed Catch2 3 when available and download it through CPM otherwise. When the
project is added with `add_subdirectory()`, tests and install rules are off by default
(`UNI_AX25_BUILD_TESTS`, `UNI_AX25_INSTALL`).
