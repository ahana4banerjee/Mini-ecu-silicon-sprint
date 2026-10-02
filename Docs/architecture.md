# MINI-ECU System & FSM Architecture Documentation

## 1. System Overview

The **MINI-ECU** (Miniature Electronic Control Unit) is a bare-metal embedded drive control system targeting the **STM32F401RET6** microcontroller (ARM Cortex-M4 @ 84 MHz). 

The primary objective of the MINI-ECU is to coordinate vehicle state management, motor propulsion speed control, serial telemetry, and instant safety interlocks without relying on a Real-Time Operating System (RTOS) or third-party framework abstractions.

---

## 2. High-Level Architecture

The system follows a modular, layered architecture separating hardware peripheral drivers from high-level state machine decisions and safety policies.

```
+-----------------------------------------------------------------------+
|                           USER INPUTS / SENSORS                       |
|   +-------------------+  +-------------------+  +-----------------+   |
|   | Touch START Sensor|  | Potentiometer (ADC)|  | EXTI E-Stop Button| |
|   +---------+---------+  +---------+---------+  +--------+--------+   |
+-------------|----------------------|---------------------|------------+
              |                      |                     |
              v                      v                     v
+-----------------------------------------------------------------------+
|                           STM32F401 MINI ECU                          |
|                                                                       |
|   +---------------------------------------------------------------+   |
|   |                  PERIPHERAL INTERFACE LAYER                   |   |
|   |   +------------+  +------------+  +-----------+  +--------+   |   |
|   |   |    GPIO    |  |  ADC1 (DMA)|  | TIM_PWM   |  | USART  |   |   |
|   |   +-----+------+  +-----+------+  +-----+-----+  +----+---+   |   |
|   +---------|---------------|---------------|-------------|-------+   |
|             |               |               |             |           |
|             v               v               v             v           |
|   +---------------------------------------------------------------+   |
|   |                APPLICATION & SAFETY FSM CORE                  |   |
|   |                                                               |   |
|   |   State Handler  <--->  Speed Mapping  <---> Command Parser   |   |
|   |   Emergency Interlock Override  <--->  Status Reporting       |   |
|   +---------------------------------------------------------------+   |
+-----------------------------------------------------------------------+
              |                      |                     |
              v                      v                     v
+-----------------------------------------------------------------------+
|                          OUTPUTS & ACTUATORS                          |
|   +-------------------+  +-------------------+  +-----------------+   |
|   | Status LEDs       |  | Motor Driver      |  | Host Serial PC  |   |
|   | (ORG, GRN, BLU, RED)| | (PWM Speed Ctrl)  |  | (115200 Baud)   |   |
|   +-------------------+  +-------------------+  +-----------------+   |
+-----------------------------------------------------------------------+
```

---

## 3. Input & Output Architecture

### Inputs:
1. **Touch START Signal:** Digital input line used to enable the system from IDLE state.
2. **Potentiometer Input:** Analog voltage source ($0\text{V} - 3.3\text{V}$) sampled by ADC to dictate commanded motor speed.
3. **Serial Command Stream (UART):** Asynchronous serial data containing textual commands (`F`, `S`, `STATUS`, `RESET`).
4. **Emergency Stop (EXTI):** Active hardware interrupt line configured to trip instantaneously on trigger.

### Outputs:
1. **Motor PWM Output:** Single-phase pulse-width modulation output governing motor speed.
2. **Status LEDs:** 4-LED status array (Orange, Green, Blue, Red) denoting active system state.
3. **Serial Telemetry (UART):** Diagnostic summaries and command execution responses transmitted back to the host system.

---

## 4. STM32F401 Mini ECU Role

The **STM32F401RET6** microcontroller serves as the central control node. Key hardware units utilized:
- **ARM Cortex-M4 Core:** Running at system clock frequency (up to 84 MHz).
- **NVIC (Nested Vectored Interrupt Controller):** Manages EXTI emergency interrupt priority over background loop processing.
- **ADC1:** Converted in 12-bit mode ($0 - 4095$) with optional DMA/polling transfer.
- **TIMx (Timer PWM):** Timer channel generating variable duty cycle PWM signal ($0\% - 100\%$).
- **USARTx:** Asynchronous serial engine configured at 115200 baud, 8 data bits, 1 stop bit, no parity.

---

## 5. FSM Architecture & State Definitions

The system behavior is governed strictly by a 4-state Finite State Machine (FSM).

```
 +------------------------------------------------------------------------+
 |                              STATE SUMMARY                             |
 +------------------+-------------------+----------------+----------------+
 | State Enum       | LED Active        | Motor Output   | Movement Allowed|
 +------------------+-------------------+----------------+----------------+
 | STATE_IDLE       | Orange            | OFF (PWM = 0)  | NO             |
 | STATE_READY      | Green             | OFF (PWM = 0)  | YES (on 'F')   |
 | STATE_RUNNING    | Blue              | ON  (PWM > 0)  | YES            |
 | STATE_EMERGENCY  | Red               | OFF (PWM = 0)  | NO             |
 +------------------+-------------------+----------------+----------------+
```

### State 1: IDLE / NOT ENABLED (`STATE_IDLE`)
- **Condition on Power-Up:** Default initial state after system boot.
- **System Behavior:** Motor is completely disabled ($0\%$ PWM).
- **LED State:** Orange LED ON, Green/Blue/Red OFF.
- **Allowed Transitions:** 
  - To `STATE_READY` upon receiving **Touch START** trigger.
  - To `STATE_EMERGENCY` upon **EXTI Emergency Stop** trigger.

### State 2: READY / ENABLED (`STATE_READY`)
- **Condition:** Touch START activated from IDLE.
- **System Behavior:** System is armed and enabled, but motor remains OFF ($0\%$ PWM) until an explicit drive command is issued.
- **LED State:** Green LED ON, Orange/Blue/Red OFF.
- **Allowed Transitions:**
  - To `STATE_RUNNING` upon receiving UART command **`F`** AND Potentiometer ADC value $> 0$.
  - To `STATE_EMERGENCY` upon **EXTI Emergency Stop** trigger.

### State 3: RUNNING (`STATE_RUNNING`)
- **Condition:** UART `F` command received in READY state with active speed selection.
- **System Behavior:** Motor PWM duty cycle actively mirrors potentiometer ADC sampling.
- **LED State:** Blue LED ON, Orange/Green/Red OFF.
- **Allowed Transitions:**
  - To `STATE_READY` upon receiving UART command **`S`**.
  - To `STATE_READY` if Potentiometer ADC speed falls to $0\%$ (configurable coast/idle behavior).
  - To `STATE_EMERGENCY` upon **EXTI Emergency Stop** trigger.

### State 4: EMERGENCY STOP (`STATE_EMERGENCY`)
- **Condition:** EXTI Interrupt triggered from ANY state.
- **System Behavior:** Immediate hardware force shutdown. Timer PWM forced to $0\%$ duty cycle immediately inside ISR. All movement commands strictly ignored.
- **LED State:** Red LED ON, Orange/Green/Blue OFF.
- **Allowed Transitions:**
  - To `STATE_IDLE` upon receiving UART command **`RESET`**. *(Note: System transitions to IDLE, requiring Touch START before reaching READY).*

---

## 6. State Transition Logic Matrix

| Current State | Trigger Event | Next State | Action Taken |
| :--- | :--- | :--- | :--- |
| **IDLE** | Touch START Input | **READY** | Turn Orange LED OFF, Green LED ON |
| **IDLE** | UART `F` | **IDLE** | Command rejected (Not enabled) |
| **READY** | UART `F` (PWM > 0) | **RUNNING** | Turn Green LED OFF, Blue LED ON, enable motor PWM |
| **READY** | UART `F` (PWM = 0) | **READY** | Command acknowledged, but speed is zero |
| **RUNNING** | UART `S` | **READY** | Set PWM = 0, Turn Blue LED OFF, Green LED ON |
| **ANY STATE**| EXTI Emergency Switch | **EMERGENCY**| Force PWM = 0, Turn Red LED ON, lock system |
| **EMERGENCY**| UART `F` or `S` | **EMERGENCY**| Command rejected |
| **EMERGENCY**| Touch START | **EMERGENCY**| Command rejected |
| **EMERGENCY**| UART `RESET` | **IDLE** | Turn Red LED OFF, Orange LED ON, clear emergency flag |

---

## 7. GPIO Architecture

GPIO pins are assigned distinct roles:
- **Output Pins:** 4x Push-Pull Outputs driving status LEDs.
- **Input Pins:** 
  - 1x Input pin configured for Touch START sensor.
  - 1x EXTI Input pin with rising/falling edge trigger for Emergency Stop.

```
PIN MAPPING — TO BE DEFINED
(Final hardware pin assignments will be specified during Phase 1 configuration)
```

---

## 8. ADC Architecture

The ADC reads the speed-command potentiometer.
- **Resolution:** 12-bit ($0 \text{ to } 4095$).
- **Sampling Mode:** Continuous or periodic timer-triggered conversion.
- **Noise Mitigation:** Simple software averaging filter (e.g., 4-sample moving average) to prevent PWM flickering.
- **Value Mapping:**
  $$\text{PWM Duty Cycle (\%)} = \frac{\text{ADC Value}}{4095} \times 100\%$$

---

## 9. PWM & Motor Drive Architecture

- **Peripheral:** General-purpose Timer (TIMx) running in PWM Mode 1.
- **Frequency:** Configured in the audible-range filter zone ($\approx 1\text{ kHz} - 20\text{ kHz}$ depending on motor driver hardware).
- **Safety Interlock:** Motor drive output is gated by FSM state. If state $\neq$ `RUNNING`, output duty cycle is forced to $0$.

---

## 10. UART Architecture

- **Baud Rate:** 115200 baud, 8N1 format.
- **Mode:** Interrupt-driven or polled RX buffer processing.
- **Command Set:**
  - `F` / `f` : Request Forward drive mode.
  - `S` / `s` : Request Stop drive mode.
  - `STATUS` : Request full diagnostic telemetry printout.
  - `RESET`  : Request emergency fault reset (clears Emergency to IDLE).
  - *(Optional: `R` / `r` for Reverse mode).*

---

## 11. EXTI Emergency Stop Architecture

Emergency shutdown utilizes dedicated External Interrupt (EXTI) hardware.
- **Trigger:** Edge-triggered interrupt line.
- **ISR Behavior:**
  1. Immediately overwrite PWM timer registers to $0\%$.
  2. Mutate global state machine flag to `STATE_EMERGENCY`.
  3. Update visual status LEDs (Red ON).
  4. Non-blocking ISR execution ($< 5\ \mu\text{s}$ latency).

---

## 12. LED Status Architecture

Visual state feedback is maintained through four dedicated indicator LEDs:

| System State | Orange LED | Green LED | Blue LED | Red LED | Description |
| :--- | :---: | :---: | :---: | :---: | :--- |
| **IDLE** | **ON** | OFF | OFF | OFF | System powered on, not enabled. |
| **READY** | OFF | **ON** | OFF | OFF | Touch START confirmed, armed for drive. |
| **RUNNING** | OFF | OFF | **ON** | OFF | Vehicle drive active, motor powered. |
| **EMERGENCY** | OFF | OFF | OFF | **ON** | Emergency stop tripped! Hardware locked. |

---

## 13. Diagnostics Architecture

When receiving the `STATUS` string over UART, the system responds with a diagnostic summary formatted as follows:

```
========================================
           MINI ECU DIAGNOSTICS         
========================================
FSM STATE    : RUNNING
SYSTEM ENABLE: ENABLED
MOTOR STATUS : ACTIVE
PWM DUTY     : 65%
ADC RAW VAL  : 2662
EMERGENCY    : NO
========================================
```

---

## 14. Software Module Structure

```
Core/
├── Inc/
│   ├── main.h             # STM32 HAL & global pins header
│   ├── ecu_fsm.h          # FSM state definitions & function prototypes
│   ├── motor_control.h    # ADC and Timer PWM motor drive abstraction
│   ├── uart_command.h     # CLI parser and telemetry reporter
│   └── gpio_indicators.h  # LED and Touch GPIO handlers
└── Src/
    ├── main.c             # Hardware init & main event loop
    ├── ecu_fsm.c          # FSM transition logic & safety rules
    ├── motor_control.c    # ADC sampling to PWM conversion implementation
    ├── uart_command.c     # Command parsing engine & UART TX handlers
    └── gpio_indicators.c  # GPIO state setters
```

---

## 15. Data & Control Flow

1. **Initialization:** Peripherals initialized $\rightarrow$ FSM enters `STATE_IDLE` $\rightarrow$ Orange LED turned ON.
2. **Main Loop Execution:**
   - Sample ADC potentiometer value continuously.
   - Poll Touch START input signal (if in `IDLE`).
   - Process incoming UART command characters.
   - Update state LED indicators.
   - Refresh PWM output (if in `RUNNING`).
3. **Interrupt Override:**
   - EXTI Emergency interrupt preempts background main loop execution at any moment.

---

## 16. Safety Behavior & Fault Handling

- **Fail-Safe Startup:** System always boots to `IDLE` with motor PWM forced to zero.
- **Fail-Safe Recovery:** A `RESET` command from `EMERGENCY` returns the system to `IDLE`, preventing unexpected motor spin until `Touch START` and `F` commands are deliberately issued again.
- **Invalid Command Rejection:** Commands issued outside their valid state context (e.g., `F` while in `IDLE` or `EMERGENCY`) are rejected and logged via serial diagnostics.

---

## 17. Integration Architecture

The integration strategy combines modular unit testing with end-to-end state transitions:
1. **Peripheral Drivers Layer:** Low-level register/HAL setup verified independently.
2. **FSM Application Core:** Independent state-machine logic driven by simulated input triggers.
3. **System Integration Layer:** Hardware event hooks bound directly to FSM state transition functions.

---

## 18. Testing Architecture

- **Unit Testing:** Individual verification of ADC-to-PWM math, LED control functions, and serial string parsing.
- **Integration Testing:** Verification of state transitions under valid and invalid sequences.
- **Safety Testing:** Deliberate injection of EXTI Emergency signals during active drive conditions to measure response time and verify lock-out behavior.

---

## 19. Hardware Pin Mapping

Target Microcontroller: **STM32F401RET6**

| Signal Name | MCU Port / Pin | Mode / Peripheral | Function / Description | Status |
| :--- | :--- | :--- | :--- | :---: |
| **ORANGE_LED** | `PB0` | GPIO Output | IDLE State Indicator LED | 🟢 **Finalized** |
| **GREEN_LED** | `PB1` | GPIO Output | READY State Indicator LED | 🟢 **Finalized** |
| **BLUE_LED** | `PB2` | GPIO Output | RUNNING State Indicator LED | 🟢 **Finalized** |
| **RED_LED** | `PB10` | GPIO Output | EMERGENCY State Indicator LED | 🟢 **Finalized** |
| **TOUCH_START** | `PA0` | GPIO Input (Pull-down) | System Enable Touch Input Sensor | 🟢 **Finalized** |
| **ESTOP_BTN** | `PC13` | EXTI15_10 Interrupt | Emergency Stop Hardware Switch | 🟡 Planned (Phase 4) |
| **POT_ADC** | `PA1` | ADC1_IN1 Analog Input | Potentiometer Speed Selection | 🟡 Planned (Phase 2) |
| **MOTOR_PWM** | `PA8` | TIM1_CH1 PWM Output | Motor Speed Control Signal | 🟡 Planned (Phase 2) |
| **UART_TX** | `PA2` | USART2_TX (AF7) | Serial Telemetry Transmit | 🟡 Planned (Phase 3) |
| **UART_RX** | `PA3` | USART2_RX (AF7) | Serial Command Receive | 🟡 Planned (Phase 3) |


---

## 20. Simulator Architecture (Optional Layer)

Simulation (using tools such as Renode or generic STM32 host models) is maintained as an **optional, non-blocking development layer**. 

The MINI-ECU software architecture relies strictly on standard ARM Cortex-M CMSIS hardware abstractions, allowing execution on physical STM32F401RET6 silicon or generic STM32F4 simulation environments without architecture modification.
