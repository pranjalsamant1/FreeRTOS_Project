# FreeRTOS Real-Time Embedded Systems Project

---

## 1. Project Overview

This project implements a **multi-task real-time embedded system** on the ESP32 using **FreeRTOS**.

The system performs concurrent monitoring, signal processing, waveform generation, and event-driven control across multiple hardware interfaces.

It demonstrates:

* Periodic real-time task scheduling
* Inter-task communication using queues
* Shared data protection using mutexes
* Analog & digital signal processing
* Frequency measurement of external signals
* Human–machine interaction via pushbutton events

---

## 2. Hardware Platform

* **MCU:** ESP32
* **RTOS:** FreeRTOS (Arduino framework integration)
* **Interfaces used:**

  * GPIO (digital I/O)
  * ADC
  * UART (Serial logging)

### I/O Mapping

| Function            | GPIO    |
| ------------------- | ------- |
| Waveform Output LED | GPIO 4  |
| Frequency Input #1  | GPIO 14 |
| Frequency Input #2  | GPIO 25 |
| Threshold LED       | GPIO 2  |
| Potentiometer (ADC) | GPIO 15 |
| Pushbutton Input    | GPIO 27 |
| Event LED Output    | GPIO 26 |

---

## 3. System Architecture

The system is decomposed into **seven concurrent FreeRTOS tasks**, grouped into functional subsystems:

### A) Signal Generation

* Generates a deterministic pulse waveform.

### B) Signal Measurement & Processing

* Measures frequencies of two external square waves.
* Stores results in shared memory.
* Scales and logs processed values.

### C) Analog Monitoring

* Samples potentiometer input.
* Applies moving average filtering.
* Drives threshold indicator.

### D) Event-Driven Human Interface

* Detects pushbutton presses.
* Sends events via queue.
* Toggles LED on each event.

---

## 4. Task Breakdown

### Task 1 — Waveform Generator

**Period:** 4 ms

Generates a deterministic digital pulse sequence:

* 200 µs HIGH
* 50 µs LOW
* 30 µs HIGH
* Remainder LOW

Demonstrates precise microsecond-scale timing within an RTOS task.

---

### Task 2 — Frequency Measurement Channel 1

**Period:** 20 ms

* Measures frequency of external square wave.
* Uses rising-edge period measurement.
* Stores result in shared struct.

---

### Task 3 — Frequency Measurement Channel 2

**Period:** 8 ms

Same method as Task 2 but operates at a higher sampling rate for a different input range.

---

### Task 4 — Analog Threshold Monitor

**Period:** 20 ms

* Reads potentiometer via ADC.
* Maintains 4-sample moving average filter.
* Compares against half-scale threshold.
* Drives LED indicator.

Implements simple real-time signal filtering.

---

### Task 5 — Data Scaling & Serial Logger

**Period:** 100 ms

* Reads both measured frequencies.
* Scales values to 0–99 range.
* Outputs formatted data via UART (9600 baud).

Represents telemetry/diagnostic reporting.

---

### Task 6 — Pushbutton Monitor (Event Producer)

**Period:** 20 ms polling

* Reads pushbutton input.
* Implements software debounce.
* Detects press events.
* Sends event messages to queue.

---

### Task 7 — Event LED Controller (Event Consumer)

* Blocks on button event queue.
* Toggles LED on each received event.

Demonstrates event-driven RTOS design with no polling CPU load.

---

## 5. Inter-Task Communication

### Shared Data Structure

```c
typedef struct {
  float freq_t2_hz;
  float freq_t3_hz;
} SharedFreqs;
```

Stores latest measured frequencies.

---

### Mutex Protection

A FreeRTOS mutex ensures safe concurrent access:

* Writers: Task 2 & Task 3
* Reader: Task 5

Prevents race conditions and inconsistent reads.

---

### Event Queue

A FreeRTOS queue connects Tasks 6 → 7:

* Producer: Button monitor
* Consumer: LED controller
* Event type: Button press notification

Enables asynchronous event handling.

---

## 6. Scheduling Strategy

Each periodic task uses:

```c
vTaskDelay(pdMS_TO_TICKS(period_ms));
```

This creates cooperative periodic execution while allowing preemption by higher-priority tasks.

Task priorities are assigned based on timing criticality:

* Higher: waveform generation, analog monitoring, button handling
* Medium: frequency measurement
* Lower: serial logging

---

## 7. Signal Processing Methods

### Frequency Measurement

* Rising-to-rising edge timing
* Microsecond resolution using `micros()`
* Timeout protection for signal loss

### Analog Filtering

* 4-sample moving average
* Reduces noise and ADC jitter
* Lightweight FIR filter implementation

---

## 8. Key RTOS Concepts Demonstrated

* Task creation & core pinning
* Periodic scheduling
* Mutex synchronization
* Queue-based messaging
* Blocking vs polling design
* Real-time timing control

---

## 9. Example Serial Output

```
34, 52
35, 51
33, 50
```

Represents scaled frequency measurements from both channels.

---

## 10. Potential Extensions

* Interrupt-based frequency capture
* DMA ADC sampling
* CAN/UART telemetry expansion
* Fault detection & watchdog integration
* AUTOSAR-style layered architecture

---

## 11. Learning Outcomes

This project demonstrates practical experience in:

* Real-time embedded firmware design
* Hardware–software integration
* Deterministic scheduling
* Concurrency management
* Event-driven system architecture

---

**Author:** Pranjal Samant
Embedded Software / Robotics Systems Engineer
# FreeRTOS_Project
