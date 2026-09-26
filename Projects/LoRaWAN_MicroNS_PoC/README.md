# LoRaWAN Micro-NS PoC

This project is a custom Network Server (NS) proof-of-concept built on top of the STM32CubeWL LoRaWAN reference stack. The goal is to keep the vendor STM32 stack as the radio and protocol base while developing a separate custom NS layer for ingress processing, payload handling, and future service orchestration.

## Project intent

The work in this repository is intentionally split into two layers:

- STM32 vendor stack: the upstream radio, LoRaWAN modem, Relay, and board-specific runtime
- Custom NS PoC: parsing and validation logic for frames observed from the Relay path, plus app-level payload handling outside the vendor service logic

This keeps the custom logic isolated and makes it easier to evolve without changing the standard Relay behavior.

## Current scope

The PoC currently includes two components:

- `Components/MicroNSIngress`
  - Relay ingress parsing and validation
  - observes the six-byte Relay metadata prefix
  - validates Join Request / data uplink framing before normal Relay forwarding continues

- `Components/AppFrameCodec`
  - compact app payload codec for simulated end-node application data
  - used to model the custom application contract separate from the LoRaWAN stack

## Repository structure

```text
Projects/LoRaWAN_MicroNS_PoC/
├── README.md
├── Components/
│   ├── MicroNSIngress/
│   │   ├── README.md
│   │   └── tests/
│   └── AppFrameCodec/
│       ├── README.md
│       ├── app_frame_codec.c
│       ├── app_frame_codec.h
│       └── tests/
└── docs/                     (reserved for future architecture docs)
```

## Maintainability principles

1. Keep vendor stack files separate from custom NS files.
2. Keep each feature in its own component folder.
3. Keep host-side validation tests close to the component that owns the logic.
4. Prefer observe-only integration at the Relay service seam until the custom NS contract is stable.
5. Do not modify the standard LoRaWAN Relay flow unless the behavior is explicitly required.

## Validation approach

The project uses lightweight host-side C tests so the logic can be checked without requiring a physical board for every iteration.

Examples:

```bash
gcc -std=c99 -Wall -Wextra -Werror \
  Components/MicroNSIngress/tests/test_micro_ns_ingress.c \
  Projects/NUCLEO-WL55JC/Applications/LoRaWAN/LoRaWAN_Relay_LBM/LoRaWAN/App/micro_ns_ingress.c \
  -I Projects/NUCLEO-WL55JC/Applications/LoRaWAN/LoRaWAN_Relay_LBM/LoRaWAN/App \
  -o micro_ns_ingress_test.exe
```

```bash
gcc -std=c99 -Wall -Wextra -Werror \
  Components/AppFrameCodec/tests/test_app_frame_codec.c \
  Components/AppFrameCodec/app_frame_codec.c \
  -o app_frame_codec_test.exe
```

## Current status

- Relay ingress parsing is implemented and validated on host-side tests.
- The custom NS logic is intentionally observe-only and non-disruptive to the Relay path.
- This PoC is the foundation for later backend/event integration and board-level verification.

## Next milestones

- Add a clean event/adapter layer from parsed ingress to a custom NS backend contract
- Define a stable end-device payload contract for app-tier processing
- Verify behavior on the STM32 target board with a real LoRaWAN link
