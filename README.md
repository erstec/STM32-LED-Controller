# Simple STM32 PWM LED Controller
## **For BlackPill/BluePill STM32F103C8 / WeAct BlackPill STM32F411CE Development boards**

# Instructions
**Build using VS Code and PlatformIO extension**
**Flash/Program using ST-Link or similar programmer**

Button input pin and PWM output pin are defined here (as long as various other parameters, which, probably, don't need to be changed):
https://github.com/erstec/STM32-LED-Controller/blob/04a7e004c747a2ec5a6655352026ff5b80c2d7fa/src/main.cpp#L11-12

- Any NO (normal opened) switch can be used.
- PWM output connected to https://www.aliexpress.com/w/wholesale-irf520-mosfet.html IRF520 MOSFET Module.
- Any DC LED / LED Strip up to 24V DC can be used, IRF520 can drive upt to 1A current without heatsink and up to 5A with.
- Corresponding DC adapter required (voltage and current should be aligned with used LED Strip).
- In case LEDs / LED strip are non-5V powered - corresponding LowPower 5V DC BEC/LDO for powering Board required (like https://www.aliexpress.com/wholesale?catId=0&SearchText=LDO+5V+DC), with output of 5V DC, connected to `VB` and any `GND` pins.

# Usage
- 15% is default after applying power
- Short press: ON / OFF (last brightness preserved)
- Long press: Gradually cycle througt 0-100%
- Double-click: Switching between 2% and 5%
- Tripple-click: Swithcing in order 10-15-20-25-50-75-100

# References
Borads are for reference, but can be used with ANY other, accordingly modified `platformio.ini`

https://stm32-base.org/boards/STM32F103C8T6-Black-Pill.html

https://stm32-base.org/boards/STM32F411CEU6-WeAct-Black-Pill-V2.0
