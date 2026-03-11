# :fire: Awesome :fire: SMT Machine

[![MIT License](https://img.shields.io/badge/license-MIT-blue.svg?style=flat)](http://choosealicense.com/licenses/mit/)
[中文文档](./README_CN.md)

Contributors:
<p float="left">
  <a href="https://github.com/baoqi-zhong"><img src="https://avatars.githubusercontent.com/u/66853126?v=4" width="50"/></a>
  <a href="https://github.com/ZimingXian"><img src="https://avatars.githubusercontent.com/u/51425827?v=4" width="50" border-radius="50%" /></a>
</p>

**Awesome SMT Machine** is an open-source Pick and Place machine project that originated from an embedded system course. The project features a fully self-developed software, hardware, and mechanical design.

It utilizes a **CoreXY** architecture for 4-DOF placement and integrates a custom **FOC (Field Oriented Control) closed-loop stepper motor driver** for high-precision and high-response positioning. The system includes a 4.2-inch color touch screen for real-time status monitoring and uses Python-based host software for coordinate file parsing and fully automatic placement.

<p float="left">
  <img src="./Docs/Image/1.jpg" width="400" />
  <img src="./Docs/Image/8.jpg" width="400" />
</p>

**Key Features**:
- **Core-XY Structure**: High precision and high-speed motion.
- **FOC Stepper Driver**: Heat-free, highly efficient, and silent operation.
- **Interactive UI**: 4.2-inch color touch screen.
- **Host Software**: Python-based control for automated workflow.

# Hardware
<p float="left">
  <img src="./Docs/Image/2.jpg" width="320" />
  <img src="./Docs/Image/7.png" width="480" />
</p>

The hardware consists of 4-layer PCBs designed using **KiCad**.

**Directory Structure**:
```
./Hardware/
    Main-Control-Board          Main control motherboard
    Stepper-Motor-FOC-Driver    FOC driver for Nema-17 stepper motors
    LCD-Board                   TFT display interface board
    Dual-Head-Motor-Driver      (Ongoing) Controller for the dual-head placement module
```

## Main Control Board
<p float="left">
  <img src="./Docs/Image/3.png" width="480" />
  <img src="./Docs/Image/5.png" width="320" />
</p>

Powered by an **STM32G431CBU6** MCU, SC8815 battery management system, and ACM8625S I2S audio amplifier.  
For details, see [Main Control Board Schematic](./Hardware/Main-Control-Board/Main-Control-Board-Schematic.pdf).

**Features**:
- **UART Interface**: Communication with the Python host software.
- **6S Battery Charger**: Supports fast charging and wide input voltage range.
- **Audio System**: Integrated amplifier and speaker (just for fun! :yum:).
- **CAN Bus**: High reliability and scalability.

## Stepper Motor FOC Driver
<p float="left">
  <img src="./Docs/Image/4.png" width="240" />
  <img src="./Docs/Image/6.png" width="560" />
</p>

Based on **STM32G431CBU6** MCU, DRV8412 gate driver, MA732 magnetic encoder, and WS2812 RGB LEDs.  
For details, see [Stepper Motor FOC Driver Schematic](./Hardware/Stepper-Motor-FOC-Driver/Stepper-Motor-FOC-Driver-Schematic.pdf).

**Features**:
- **Custom FOC Algorithm**: Three-loop control (Torque, Speed, Angle). Implements DQ-axis decoupling, BEMF feedforward, cogging torque compensation, and dead-time compensation.
- **Precision Control**: Real current loop ensures precise torque control.
- **14-Bit Encoder**: High-speed magnetic encoder with automatic calibration and adaptive notch filter (ANF) for low noise.
- **CAN Bus**: Bi-directional communication.

# Software
The embedded software is developed using **STM32CubeMX**, **Makefile**, and **GCC**. The FOC algorithm is implemented with a bare-metal interrupt-driven architecture for maximum performance. The main control board utilizes **FreeRTOS** for multitasking (communication, motion control, battery management, audio playback, etc.).

**Directory Structure**:
```
./Software/
    ClosePnP                        Python/tkinter-based host control software (inspired by OpenPnP)
    Main-Control-Board-Software     STM32 firmware for the main board
    Stepper-Motor-Driver-Software   FOC algorithm source code
    MusicConverter                  Python script to convert .WAV files to C arrays for audio testing
    Dual-Head-Motor-Driver-Software (Ongoing) Firmware for the dual-head motor driver
```

# Mechanical
The original single-head version was designed using **Cinema 4D (C4D)**. The newer dual-head version is designed with **SolidWorks**.

- **Single-head source files**: `./Mechanical/CoreXY.c4d` (or `./Mechanical/CoreXY.stl`).
