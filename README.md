# Space Rover FreeRTOS Architecture

A portfolio-grade starter project for a planetary rover split between:

- **STM32 + FreeRTOS** for deterministic sensing, motor control, safety and communications
- **micro-ROS** for MCU-to-Linux communication
- **ROS 2** for supervision, visualization, navigation and autonomy

## System architecture

```text
IMU / encoders / battery / motors
              |
        STM32 + FreeRTOS
              |
     task notifications + queues
              |
           micro-ROS
              |
      ROS 2 agent on Linux
              |
 telemetry + control + Nav2 + SLAM
```

## Included

- Portable C firmware architecture
- FreeRTOS tasks, queues, event bits and timing configuration
- Host-buildable mock platform for early testing
- Starter ROS 2 message, control and telemetry packages
- FreeRTOS internal architecture guide
- Safety concept and verification plan
- GitHub Actions starter CI
- Bash setup, build, test and Git initialization scripts
- CubeMX integration instructions

## Not included intentionally

The STM32 HAL, startup code, linker script, CMSIS files, FreeRTOS kernel and generated
micro-ROS library are board/toolchain-specific. Generate or import them after choosing
your MCU board. This repository keeps application logic independent of generated code.

## Quick start: host validation

```bash
chmod +x scripts/*.sh
./scripts/setup_ubuntu.sh
./scripts/build_host.sh
./build-host/rover_host_demo
./scripts/run_tests.sh
```

## Quick start: ROS 2 workspace

After installing ROS 2 Jazzy:

```bash
./scripts/build_ros2.sh
source ros2_ws/install/setup.bash
ros2 run rover_control rover_control_node
```

## STM32 integration

1. Select an STM32 board and create a CubeMX project.
2. Enable FreeRTOS, GPIO, timers, UART, I2C/SPI and required DMA channels.
3. Copy or reference `firmware/app`, `firmware/drivers` and `firmware/platform`.
4. Implement functions declared in `platform/rover_hal.h`.
5. Call `rover_app_init()` before starting the scheduler.
6. Call `rover_create_tasks()` before `vTaskStartScheduler()`.
7. Integrate micro-ROS only after local tasks and safety behavior are tested.

See `docs/STM32_CUBEMX_INTEGRATION.md`.

## First Git commands

```bash
git init
git add .
git commit -m "chore: initialize rover FreeRTOS architecture"
git branch -M main
git remote add origin https://github.com/YOUR_USERNAME/space-rover-freertos-architecture.git
git push -u origin main
```

## Development milestones

- [x] Initial architecture and host mocks
- [ ] Select STM32 board and pin mapping
- [ ] Bring up IMU and encoder drivers
- [ ] Validate 100 Hz motor control task
- [ ] Add micro-ROS transport and messages
- [ ] Build rover URDF/Xacro and Gazebo model
- [ ] Add hardware-in-the-loop testing
- [ ] Integrate SLAM and Nav2
- [ ] Add fault injection, watchdog and safe-state tests

## Safety notice

This is an educational starter architecture, not flight-qualified or safety-certified
software. Validate motor limits, emergency stop behavior, watchdogs and electrical
protection before connecting physical actuators.
