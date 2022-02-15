# STM32F103x6-Modified-Homework-Project

A simple STM32F103x6 microcontroller project made using Proteus, CubeMX and FreeRTOS. Two buttons for motor speed controlling, increasing and decreasing. Temperature ADC used for LED blinking at certain intervals. Keyboard, 7seg, LCD, UART implementations and more.

Eclipse Settings:
Add STM32F103x6 and USE_HAL_DRIVERS as GNU C Symbols.
Add path to arm-none-eabi-gcc toolchain->bin dir and make-gnu bin dir.
Add -fdebug-prefix-map==../ in makefile.
Add -mlong-calls in makefile.

Run Proteus as Admin!
