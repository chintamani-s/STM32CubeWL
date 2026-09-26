# Application Frame Codec

This component composes and parses the simulated end-node application payload. It operates on application bytes after LoRaWAN decapsulation and payload decryption; LoRaWAN frame parsing, MIC verification, frame counters, and radio relay behavior remain outside this component.

## Wire format

The frame is exactly 6 bytes, with multi-byte values in big-endian order:

| Bytes | Field | Encoding |
| --- | --- | --- |
| 0-1 | `temperature_c_x100` | Signed 16-bit, two's-complement, 0.01 C units |
| 2 | `button_state` | `0` or `1` |
| 3 | `battery_percent` | `0` through `100` |
| 4-5 | `status_flags` | Unsigned 16-bit bit field |

The layout is a PoC contract based on the proposal's `sensor_payload_t`; it is not asserted to be an existing ST or LoRaWAN standard format. Update it only after the end-node payload requirements are agreed.

## Host test

From the repository root, compile and run `tests/test_app_frame_codec.c` with `app_frame_codec.c` using a C99 compiler and warnings-as-errors.