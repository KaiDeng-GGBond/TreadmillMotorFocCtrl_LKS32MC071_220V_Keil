# Treadmill Motor FOC Controller

FOC motor controller for treadmill based on LKS32MC071.

---

## Project Information

| Item | Description |
|------|-------------|
| MCU | LKS32MC071C8T8 |
| IDE | Keil MDK V5 |
| Language | C |
| Motor | PMSM |
| Pole pairs | 5 |
| Pole | 10 |
| Driver | EG2336 |
| Control | FOC |
| Encoder | HALL |
| Communication | UART |

---

## Features

- [x] FOC Current Loop
- [x] Speed Loop
- [x] UART Communication
- [x] Current Protection

---

## Directory Structure

```text
.
 0_Include
 1_LKS_FwLib
 2_HardwareDriverLayer
 3_CommonServiceLayer
 4_MotorDriveLayer
 5_MotorAppLayer
 6_UserAppLayer
 Docs
 MDK-ARM
```

---

## Build

Open

```text
MDK-ARM/LK_StdPeriph.uvprojx
```

Compile with

```
Keil MDK V5
```

---

## Development Environment

| Tool | Version |
|------|----------|
| Keil MDK | V5 |
| Git | Latest |
| GitHub | Latest |

---

## Version History

| Version | Date | Description |
|---------|------|-------------|
| v1.0.0 | 2026-07-02 | Initial Release |

---
