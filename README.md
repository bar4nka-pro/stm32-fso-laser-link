README IS !NOT! READY

that just text for other people tbh

[![CI](https://github.com/bar4nka-pro/stm32-fso-laser-link/actions/workflows/ci.yml/badge.svg)](https://github.com/bar4nka-pro/stm32-fso-laser-link/actions/workflows/ci.yml)
```
PC terminal
    │  USB
    ▼
STM32 #1 (transmitter)
    │  USART1 TX (PA9) → driver → LASER
    ▼
  ~~~ air, 650 nm light ~~~
    ▼
PHOTODIODE → Schmitt trigger → USART1 RX (PA10)
STM32 #2 (receiver)
    │  USB
    ▼
PC terminal
```
