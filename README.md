# MINI-ECU: Bare-Metal Embedded Drive Control System

[![Hackathon](https://img.shields.io/badge/Hackathon-Silicon%20Sprint%20by%20Maven%20Silicon-blue)](https://www.maven-silicon.com/)
[![Target MCU](https://img.shields.io/badge/MCU-STM32F401RET6-red)](https://www.st.com/)
[![Architecture](https://img.shields.io/badge/Architecture-Bare--Metal%20C%20%7C%20FSM-green)](#system-architecture-overview)
[![Status](https://img.shields.io/badge/Phase%200-Foundation%20In%20Progress-yellow)](#current-project-status)

---

## 📌 Project Overview

**MINI-ECU** is a bare-metal embedded drive control unit designed for the **Silicon Sprint Hackathon by Maven Silicon**. Built around the **STM32F401RET6** ARM Cortex-M4 microcontroller, the project demonstrates safe, deterministic, and real-time coordination of motor propulsion, user enabling signals, speed selection, serial diagnostics, and instant hardware emergency shutdown.

The core challenge of this project lies in embedded software engineering—crafting a robust **Finite State Machine (FSM)** in pure bare-metal C without high-level RTOS abstractions or external robotic libraries.

---

## 🎯 Problem Overview & Core Objectives

Modern Electronic Control Units (ECUs) in automotive and industrial drives require strict execution ordering, continuous hardware health monitoring, and instant safety interlocks. 

The objective of the **MINI-ECU** is to manage a motorized drive platform with the following core responsibilities:
1. **Deterministic State Control:** Ensure movement cannot occur unless explicitly enabled by user touch input and validated commands.
2. **Speed Selection:** Sample analog potentiometer inputs via ADC and translate them into motor PWM duty cycles.
3. **Serial Command & Diagnostics:** Accept UART commands (`F`, `S`, `STATUS`, `RESET`) while reporting live system health metrics.
4. **Instant Emergency Interlock:** Utilize external interrupts (EXTI) for hardware emergency shutdown with fail-safe recovery semantics.
5. **Visual State Indication:** Drive designated status LEDs representing specific state conditions.

---

## ⚙️ Core System Concept & FSM Overview

The system operates strictly according to a deterministic Finite State Machine (FSM).

```
                      +-------------------+
                      |     POWER ON      |
                      +-------------------+
                                |
                                v
                      +-------------------+
                      | IDLE / NOT ENABLED| (Orange LED ON)
                      +-------------------+
                                |
                         [ Touch START ]
                                |
                                v
                      +-------------------+
                      |   READY / ENABLED | (Green LED ON)
                      +-------------------+
                                |
                     [ UART F & PWM > 0 ]
                                |
                                v
                      +-------------------+
                      |      RUNNING      | (Blue LED ON)
                      +-------------------+
                                |
                            [ UART S ]
                                |
                                v
                      +-------------------+
                      |   READY / ENABLED | (Green LED ON)
                      +-------------------+

===================================================================
                       EMERGENCY STOP (ANY STATE)
===================================================================
                             ANY STATE
                                 |
                        [ EXTI Emergency ]
                                 |
                                 v
                      +-------------------+
                      |  EMERGENCY STOP   | (Red LED ON, PWM = 0)
                      +-------------------+
                                 |
                            [ UART RESET ]
                                 |
                                 v
                      +-------------------+
                      | IDLE / NOT ENABLED| (Requires Touch START)
                      +-------------------+
```

### Core State Logic Rules:
- **IDLE / NOT ENABLED:** Motor is disabled (PWM = 0). Movement commands are strictly ignored.
- **READY / ENABLED:** Motor remains OFF until a valid forward command (`F`) is received with active PWM.
- **RUNNING:** Vehicle moves according to analog potentiometer speed.
- **EMERGENCY STOP:** Interrupt-driven instantly from any state. Disables motor PWM immediately.
- **RESET Semantics:** Issuing a `RESET` command clears the emergency condition and transitions to **IDLE**, requiring a manual **Touch START** before movement can resume. `RESET` does NOT directly start the motor.

---

## 🚦 System State & LED Status Table

| State | Motor Status | Orange LED | Green LED | Blue LED | Red LED |
| :--- | :--- | :---: | :---: | :---: | :---: |
| **IDLE / Not Enabled** | OFF (PWM = 0) | **ON** | OFF | OFF | OFF |
| **READY / Enabled** | OFF (PWM = 0) | OFF | **ON** | OFF | OFF |
| **RUNNING** | Active PWM | OFF | OFF | **BLUE** | OFF |
| **EMERGENCY** | OFF (PWM = 0) | OFF | OFF | OFF | **RED** |

---

## 🛠️ Hardware & Peripheral Requirements

The target platform for the Mini-ECU is the **STM32F401RET6** microcontroller (Nucleo-F401RE board family).

- **GPIO:** Visual indicator status LEDs (Orange, Green, Blue, Red) and Touch START input sensor signal.
- **ADC:** 12-bit Analog-to-Digital Converter sampling potentiometer position for speed reference.
- **Timer / PWM:** General-purpose timer configured in PWM generation mode for motor drive control.
- **UART:** Asynchronous serial communication interface (115200 baud, 8N1) for command receiving and telemetry output.
- **EXTI (External Interrupt):** Dedicated interrupt line connected to an Emergency Stop push-button/switch for immediate hardware override.

> 📌 **Hardware Pin Mapping:** Status LEDs (PB0, PB1, PB2, PB10) and Touch START (PA0) are **Finalized** in `Core/Inc/board_config.h`. Peripheral instances (TIM1 PWM, ADC1_IN1, USART2) are allocated for Phase 2–4.

---

## 💻 Software & Toolchain Requirements

- **Language:** Bare-Metal C (C99 standard).
- **IDE:** STM32CubeIDE (v1.14.0 or compatible).
- **Compiler:** `arm-none-eabi-gcc` cross-compiler toolchain.
- **Driver Layer:** STM32F4 HAL / Low-Layer (LL) / CMSIS bare-metal register interaction.
- **Strictly Prohibited:** RTOS (FreeRTOS, ThreadX), Arduino Framework, or high-level robotic packages.

---

## 📁 Repository Structure

```
mini-ecu-silicon-sprint/
│
├── Core/
│   ├── Inc/               # Application & peripheral header files
│   └── Src/               # Core application logic & main entry
│
├── Drivers/               # STM32F4xx HAL and CMSIS driver packages
│
├── Docs/                  # Project documentation & design artifacts
│   ├── architecture.md    # System & FSM technical architecture specification
│   └── roadmap.md         # Phase-by-phase implementation & verification plan
│
├── Simulation/            # Simulator definitions (Renode / testing configs)
│
├── README.md              # Main project readme & quickstart
├── .gitignore             # Standard Git ignore rules for STM32CubeIDE
├── .project               # STM32CubeIDE project definition
├── .cproject              # STM32CubeIDE C/C++ build configuration
└── Mini-ecu-silicon-sprint.ioc  # STM32CubeMX graphical configuration file
```

---

## 👥 Team Structure & Responsibilities

| Member | Focus Area | Core Responsibilities |
| :--- | :--- | :--- |
| **Member 1** | **FSM & System Architecture** | Application state machine manager, transition rules, integration safety logic, state machine architecture. |
| **Member 2** | **ADC, PWM & Motor Drive** | Potentiometer sampling, ADC conversion filtering, timer PWM output generation, motor driver abstraction. |
| **Member 3** | **UART & Serial Diagnostics** | Serial RX/TX driver, CLI command parser (`F`, `S`, `STATUS`, `RESET`), status report formatting. |
| **Member 4** | **GPIO, Touch, EXTI & Testing** | GPIO pin initializations, status LED control, Touch START integration, EXTI emergency handler, simulator/test harness. |

---

## 📊 Current Project Status

- [x] **Phase 0 — Project Foundation** *(In Progress)*
  - [x] GitHub repository initialized
  - [x] STM32CubeIDE project created (`STM32F401RET6`)
  - [x] Folder structure established
  - [x] Architecture & Roadmap documentation written
  - [ ] Peripheral pin assignment finalized *(Pending Hardware Phase 1)*
- [ ] **Phase 1 — GPIO + System Foundation + FSM** *(Not Started)*
- [ ] **Phase 2 — ADC + PWM + Speed Control** *(Not Started)*
- [ ] **Phase 3 — UART + Command Processing + Diagnostics** *(Not Started)*
- [ ] **Phase 4 — EXTI + Emergency Stop + Recovery** *(Not Started)*
- [ ] **Phase 5 — Full System Integration** *(Not Started)*
- [ ] **Phase 6 — Validation + Final Demonstration + Documentation** *(Not Started)*

---

## 🧪 Testing & Simulation Strategy

1. **Hardware Verification:** Incremental module testing using oscilloscope/logic analyzer for PWM duty cycle accuracy, EXTI latency, and UART signal integrity.
2. **Simulation Layer (Optional/Auxiliary):** System architecture is completely simulator-independent. Renode or generic STM32F4 simulation target environments may be utilized during host testing phase for basic logic verification.
3. **Edge Case Validation:** Rigorous state violation testing (e.g., sending `F` while in Emergency, changing potentiometer while in IDLE, rapid `RESET` signaling).

---

## 🚀 Expected Final Demonstration Workflow

1. **Startup:** Power ON $\rightarrow$ Orange LED ON $\rightarrow$ Motor OFF.
2. **Enable:** Touch START signal $\rightarrow$ Orange LED OFF, Green LED ON.
3. **Speed & Drive:** Adjust Potentiometer $\rightarrow$ Send `F` over UART $\rightarrow$ Motor spins with Blue LED ON. Adjusting potentiometer dynamically alters speed.
4. **Normal Stop:** Send `S` over UART $\rightarrow$ Motor stops smoothly, system returns to READY (Green LED ON).
5. **Emergency Stop:** Press EXTI Emergency switch while running $\rightarrow$ Motor immediately halts (PWM = 0), Red LED turns ON.
6. **Recovery:** Send `RESET` over UART $\rightarrow$ System enters IDLE (Orange LED ON). Press Touch START $\rightarrow$ System enters READY (Green LED ON). Send `F` $\rightarrow$ Drive resumes.
7. **Diagnostics:** Send `STATUS` over UART at any time $\rightarrow$ Displays clean breakdown of current FSM state, motor state, active PWM percentage, raw ADC readings, and safety flags.

---

## 🔮 Future & Optional Extensions

- Optional Reverse (`R`) directional movement command.
- Software low-pass filtering on ADC potentiometer readings to reduce speed jitter.
- Controller Area Network (CAN) bus message reporting.
- Non-volatile storage of fault logs in flash memory.

---

## 📄 License & Attribution

Developed for the **Silicon Sprint Hackathon** organized by **Maven Silicon**.  
*Target Hardware:* STMicroelectronics STM32F401RET6 (ARM Cortex-M4).
