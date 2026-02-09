# FreeRTOS Real-Time Embedded Systems Project

---

## 1. Project Overview

* This project implements a **multi-task real-time embedded system** on the ESP32 using **FreeRTOS**.

* The system performs monitoring, signal processing, waveform generation, and event-driven control.

Key things It demonstrates:

* Real-time task scheduling
* Inter-task communication using queues
* Shared data protection using mutexes
* Human–machine interaction via pushbutton events


---

## 4. Task Breakdown

### Task 1 — Waveform Generator

**Period:** 4 ms

Generates digital pulse sequence:

* 200 µs HIGH
* 50 µs LOW
* 30 µs HIGH
* Remainder LOW

---

### Task 2 — Frequency Measurement Channel 1

**Period:** 20 ms

* Measures frequency of external square wave.
* Stores result in shared struct.

---

### Task 3 — Frequency Measurement Channel 2

**Period:** 8 ms

Same method as Task 2.

---

### Task 4 — Analog Threshold Monitor

**Period:** 20 ms

* Reads potentiometer via ADC.
* Maintains 4-sample moving average filter.
* If avarage > halfscale LED turns on.

---

### Task 5 — Data Scaling & Serial Logger

**Period:** 100 ms

* Reads both measured frequencies from task 2, and task 3.
* Scales values to 0–99 range.
* Outputs formatted data via UART (9600 baud).

---

### Task 6 — Pushbutton Monitor (Event Producer)

**Period:** 20 ms polling

* Reads pushbutton input.
* Sends event messages to queue.

---

### Task 7 — Event LED Controller (Event Consumer)

* Blocks on button event queue.
* Toggles LED on each received event.

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

A FreeRTOS mutex ensures safe access between below tasks:

* Writers: Task 2 & Task 3
* Reader: Task 5

Prevents race conditions and inconsistent reads.

---

### Event Queue

A FreeRTOS queue connects Tasks 6 → 7:

* Producer: Button monitor
* Consumer: LED controller
* Event type: Button press notification

---

## 6. Scheduling Strategy

Each periodic task uses:

```c
vTaskDelay(pdMS_TO_TICKS(period_ms));
```

Task priorities are assigned based on timing criticality:

* Higher: waveform generation, analog monitoring, button handling
* Medium: frequency measurement
* Lower: serial logging

---

## 7. Signal Processing Methods

### Frequency Measurement

* Calculated time between two edges
* Used micros() for microsecond timing
* Added timeout if signal is missing

### Analog Filtering

* Read potentiometer using ADC
* Took average of last 4 readings
* Helped reduce noise and fluctuations
* Compared average with threshold to control LED

---

## 10. Potential Extensions

* Interrupt-based frequency capture

---

---
## 2. Hardware Platform

* **MCU:** ESP32
* **RTOS:** FreeRTOS (Arduino framework integration)
* **Interfaces used:**

  * GPIO (digital I/O)
  * ADC
  * UART (Serial logging)

---

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


**Author:** Pranjal Samant
Embedded Software / Robotics Systems Engineer
# FreeRTOS_Project
