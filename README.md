
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
