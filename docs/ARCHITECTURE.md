# Architecture decisions

- MCU owns deterministic control and safety.
- Linux owns high-level autonomy.
- Hardware is hidden behind `rover_hal.h`.
- Safety logic is pure C and host-testable.
- ROS-facing and MCU-facing interfaces are versioned.
