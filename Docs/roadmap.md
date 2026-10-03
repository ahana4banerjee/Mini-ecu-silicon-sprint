# MINI-ECU Project Implementation Roadmap

---

## 📌 Status Summary

| Phase | Description | Status | Target Completion |
| :--- | :--- | :---: | :---: |
| **Phase 0** | Project Foundation & Architecture Documentation | 🟢 **COMPLETED** | Current |
| **Phase 1** | GPIO + System Foundation + FSM | 🟢 **COMPLETED** | Phase 1 Sprint |
| **Phase 2** | ADC + PWM + Speed Control | 🟢 **COMPLETED** | Phase 2 Sprint |
| **Phase 3** | UART + Command Processing + Diagnostics | ⚪ **NOT STARTED** | Phase 3 Sprint |
| **Phase 4** | EXTI + Emergency Stop + Recovery | ⚪ **NOT STARTED** | Phase 4 Sprint |
| **Phase 5** | Full System Integration | ⚪ **NOT STARTED** | Phase 5 Sprint |
| **Phase 6** | Validation + Final Demonstration + Documentation | ⚪ **NOT STARTED** | Hackathon Final |

---

## PHASE 0 — Project Foundation

### Objective
Establish the repository foundation, directory layout, target toolchain configuration, project structure, and comprehensive architecture specifications.

### Tasks
- [x] Create GitHub repository (`mini-ecu-silicon-sprint`)
- [x] Initialize STM32CubeIDE project for `STM32F401RET6` target MCU
- [x] Create standard folder structure (`Core/`, `Drivers/`, `Docs/`, `Simulation/`)
- [x] Author comprehensive project `README.md`
- [x] Author technical system architecture document (`Docs/architecture.md`)
- [x] Author development roadmap document (`Docs/roadmap.md`)
- [x] Finalize physical hardware pin mapping table (`Core/Inc/board_config.h`)

### Expected Deliverables
- Fully initialized repository structure
- Project design & architecture documentation suite

### Testing & Completion Criteria
- Project builds cleanly in STM32CubeIDE without warnings/errors.
- Repository structure adheres strictly to project guidelines.

### Responsible Team Member(s)
- **All Team Members (Member 1, Member 2, Member 3, Member 4)**

---

## PHASE 1 — GPIO + System Foundation + FSM

### Objective
Configure GPIO peripherals, implement Touch START input sampling, drive status LEDs, and establish the core Finite State Machine (FSM) managing `IDLE` and `READY` states.

### Tasks
- [x] Define physical pin assignments for status LEDs and Touch START sensor (`board_config.h`)
- [x] Initialize GPIO output pins for Orange, Green, Blue, and Red status LEDs (`GPIOB`: PB0, PB1, PB2, PB10)
- [x] Initialize GPIO input pin for Touch START input sensor (`GPIOA`: PA0)
- [x] Implement core FSM state manager structure and enum states (`ecu_fsm.h`)
- [x] Implement `IDLE` state behavior (Orange LED ON, Motor disabled) (`ecu_fsm.c`)
- [x] Implement `READY` state behavior (Green LED ON, Motor disabled) (`ecu_fsm.c`)
- [x] Implement FSM transition logic from `IDLE` $\rightarrow$ `READY` on Touch START trigger (`ecu_fsm.c`)
- [x] Implement visual LED verification routines

### Expected Deliverables
- `gpio_indicators.c` / `gpio_indicators.h`
- `ecu_fsm.c` / `ecu_fsm.h` (Initial FSM state engine)
- Verified `IDLE` and `READY` state transitions driven by Touch START input

### Testing Criteria
- Booting system lights Orange LED exclusively (`IDLE` state).
- Activating Touch START input transitions system to Green LED exclusively (`READY` state).
- Motor output remains zero during both states.

### Completion Criteria
- Stable FSM state engine transitioning cleanly between `IDLE` and `READY` with correct LED outputs.

### Responsible Team Member(s)
- **Member 1 (FSM Architecture)**
- **Member 4 (GPIO & Touch Sensor)**

---

## PHASE 2 — ADC + PWM + Speed Control

### Objective
Configure Timer PWM and ADC peripherals to sample potentiometer position and continuously translate analog readings into proportional motor speed PWM duty cycles.

### Tasks
- [x] Select Timer instance and pin channel for PWM output generation (`TIM1_CH1` on `PA8`)
- [x] Configure Timer PWM mode (target frequency: $1\text{ kHz}$)
- [x] Select ADC channel for Potentiometer analog input pin (`ADC1_IN1` on `PA1`)
- [x] Configure ADC 12-bit conversion mode
- [x] Develop ADC sampling and noise filtering (deadband filtering in `ADC_To_PWM_Percent`)
- [x] Develop conversion function mapping 12-bit ADC value ($0 - 4095$) to PWM Duty Cycle ($0\% - 100\%$)
- [x] Integrate motor speed control into `RUNNING` state
- [x] Guarantee $0\%$ PWM output whenever state is not `RUNNING`

### Expected Deliverables
- `motor_control.c` / `motor_control.h`
- ADC sampling driver and ADC-to-PWM mapping module
- PWM generation timer setup

### Testing Criteria
- Potentiometer min position yields $0\%$ PWM duty cycle; max position yields $100\%$ duty cycle.
- Intermediate positions (25%, 50%, 75%) yield matching linear PWM duty cycles.
- PWM output is completely silenced ($0\%$ duty cycle) when system is in `IDLE` or `READY`.

### Completion Criteria
- Smooth motor speed regulation proportional to potentiometer position when system is armed.

### Responsible Team Member(s)
- **Member 2 (ADC, PWM & Motor Control)**

---

## PHASE 3 — UART + Command Processing + Diagnostics

### Objective
Configure UART peripheral to receive PC terminal commands (`F`, `S`, `STATUS`, `RESET`) and output real-time system state and diagnostic reports.

### Tasks
- [ ] Select USART peripheral instance and baud rate configuration (115200 8N1)
- [ ] Implement serial receive buffer and command parser module
- [ ] Implement command handler for **`F`** (Forward Drive Request)
- [ ] Implement command handler for **`S`** (Stop Request)
- [ ] Implement command handler for **`STATUS`** (Diagnostic Summary Output)
- [ ] Implement command handler for **`RESET`** (Fault Clear Request)
- [ ] Implement invalid command detection and rejection messaging
- [ ] Integrate `F` and `S` commands into FSM state engine (`READY` $\leftrightarrow$ `RUNNING`)

### Expected Deliverables
- `uart_command.c` / `uart_command.h`
- CLI command parser & diagnostic reporting engine

### Testing Criteria
- Transmitting `F` over serial while in `READY` with PWM $>0$ transitions state to `RUNNING` (Blue LED ON).
- Transmitting `F` while in `IDLE` is rejected.
- Transmitting `S` while in `RUNNING` returns system to `READY` (Green LED ON).
- Transmitting `STATUS` prints a well-formatted system status table over serial.

### Completion Criteria
- Complete command interaction verified via serial terminal emulator without lockups.

### Responsible Team Member(s)
- **Member 3 (UART & Diagnostics)**

---

## PHASE 4 — EXTI + Emergency Stop + Recovery

### Objective
Configure External Interrupt (EXTI) hardware for instant Emergency Stop execution and establish safe recovery semantics.

### Tasks
- [ ] Select dedicated EXTI hardware pin for Emergency Stop push-button
- [ ] Configure EXTI edge trigger (Rising/Falling edge) and NVIC priority
- [ ] Implement EXTI Interrupt Service Routine (ISR)
- [ ] Force Timer PWM to $0\%$ duty cycle immediately inside EXTI ISR
- [ ] Mutate state machine to `EMERGENCY` state and light Red LED
- [ ] Block all movement commands while in `EMERGENCY` state
- [ ] Implement `RESET` recovery handler transitioning `EMERGENCY` $\rightarrow$ `IDLE`
- [ ] Verify that `RESET` requires Touch START before returning to `READY`

### Expected Deliverables
- EXTI interrupt handler in `main.c` / `ecu_fsm.c`
- Immediate hardware emergency shutdown mechanism
- Emergency fault recovery handler

### Testing Criteria
- Triggering EXTI hardware line from ANY state instantly shuts down motor ($0\%$ PWM) and lights Red LED.
- Movement commands (`F`) are strictly ignored during `EMERGENCY`.
- Transmitting `RESET` command clears Red LED and transitions to `IDLE` (Orange LED ON).
- Vehicle does NOT start directly on `RESET`.

### Completion Criteria
- Deterministic, instant emergency override with verified safe multi-step recovery.

### Responsible Team Member(s)
- **Member 4 (GPIO & EXTI)**
- **Member 1 (FSM Architecture & Safety)**

---

## PHASE 5 — Full System Integration

### Objective
Integrate all hardware modules (GPIO, ADC, PWM, UART, EXTI) with the core state machine and execute end-to-end integration and edge-case testing.

### Tasks
- [ ] Combine all sub-modules into `main.c` event loop
- [ ] Verify hardware interrupt priorities (EXTI > UART > System Tick)
- [ ] Conduct comprehensive state transition matrix testing
- [ ] Test edge cases:
  - `F` command issued prior to Touch START
  - `F` command issued with $0\%$ potentiometer position
  - `F` command issued during active Emergency
  - Emergency switch triggered during active `RUNNING` state
  - Rapid potentiometer manipulation during `RUNNING` state
  - Multiple Touch START trigger presses
  - Invalid UART strings
  - `RESET` issued while system is not in Emergency state

### Expected Deliverables
- Integrated, production-ready bare-metal C firmware codebase
- Comprehensive test results log

### Testing Criteria
- 100% pass rate across all state transition matrix tests and edge cases.
- Zero unexpected hardware hangs or state lockups.

### Completion Criteria
- Complete, cohesive firmware operating predictably under all test scenarios.

### Responsible Team Member(s)
- **All Team Members (Led by Member 1)**

---

## PHASE 6 — Validation + Final Demonstration + Documentation

### Objective
Perform final hardware validation, finalize code formatting and documentation, and record/prepare the hackathon presentation.

### Tasks
- [ ] Clean code repository (remove temporary debug prints, format indentation)
- [ ] Finalize inline code comments and documentation headers
- [ ] Update `Docs/architecture.md` with final physical pin mapping table
- [ ] Execute standard 7-step hackathon demonstration workflow
- [ ] Record demonstration video / capture logic analyzer telemetry traces
- [ ] Finalize hackathon submission package

### Expected Deliverables
- Clean, documented GitHub repository
- Final demonstration video / telemetry traces
- Complete submission materials

### Testing Criteria
- Flawless live execution of full 7-step demonstration sequence.

### Completion Criteria
- Project successfully submitted for Silicon Sprint Hackathon evaluation.

### Responsible Team Member(s)
- **All Team Members**
