# Industrial Grade Low Power General Purpose Microcontrollers CH32L103
EN | [中文](README_zh.md)



### Overview
CH32L103 is an industrial-grade low-power general-purpose microcontroller designed based on barley RISC-V core.CH32L103 built-in USB and PD PHY, support for PDUSB, including USB Host host and USB Device device function, USB PD and Type-C fast charging function, built-in low-power timer, provides a group of OPA op-amps, 3 groups of CMP voltage comparator, 4 groups of USART serial interface, 2 I2C interfaces, 2 SPI interfaces, 1 CAN interface, multiple timers, 12-bit ADC, 10-way Touchkey and other rich peripheral resources.

### System Block Diagram

![image-20240509144136485](C:\Users\DZG\AppData\Roaming\Typora\typora-user-images\image-20240509144136485.png)


### Features
- Barley 32-bit RISC-V4C core
- Supports RV32IMAC instruction set and self-expanding instructions

- Fast programmable interrupt controller + hardware interrupt stack
- Branch prediction, conflict handling mechanism
- Single-cycle multiplication, hardware division
- System frequency 96MHz
- Maximum 20KB volatile data storage SRAM
- CodeFlash 64KB program memory max.
- 3328B BootLoader system bootloader storage area
- 256B system non-volatile configuration information storage area
- 256B user-defined information storage area
-  System power supply VDD rating: 3.3V
- Low power modes: sleep, stop, standby
- VBAT power supply independently for RTC and backup registers.
- Built-in factory-tuned 8MHz RC oscillator
- Built-in RC oscillator of about 40KHz.
- Built-in PLL with optional CPU clock up to 96MHz.
- External support for high speed oscillator from 3 to 25MHz
- External support for 32.768KHz low-speed oscillator
- Power on/off reset, programmable voltage monitor
- Real-time clock RTC: 32-bit independent timer
- 8-way general-purpose DMA controller: 8 channels, support ring buffer management
- DMA support TIMx/ADC/USART/I2C/SPI
- 3 analog voltage comparators CMP: 2 input channels each, optional common reference voltage pin, optional output to I/O or internal direct trigger TIM2
- 1 group op-amp OPA/PGA/voltage comparator: multiple input channels, selectable multi-step gain, multiple output channels, selectable ADC pins
- 12-bit analog-to-digital ADC: analog input range VSSA ~ VDDA
- 10 external signal channels + 2 internal signal channels
- On-chip temperature sensor
- 10 TouchKey channels detection
- 1 x 16-bit low-power timer
- 1 x 16-bit advanced timer providing deadband control and emergency brake, complementary PWM outputs for motor control
- 2 x 16-bit general purpose timers providing input capture/output compare/PWM/pulse count and incremental encoder inputs
- 1 32-bit general purpose timer
- 2 x Watchdog Timers: Standalone and Windowed
- System time base timer: 64-bit counter
- 4 x USART serial ports: supports LIN and ISO7816
- 2 x I2C interfaces: SMBus/PMBus support
- 2 x SPI interfaces
- 1 group CAN interface (2.0B active): support CANFD protocol
- USB2.0 Full Speed Controller and PHY: support USB host or USB device
- USB PD and Type-C controller and PHY: support DRP, Sink and Source applications, support PDUSB
-  37 I/O ports, support 16 external interrupts
- Security features: chip unique ID
- Debug mode: Serial 2-wire debug interface SDI
- Packages: LQFP, QFN, QSOP and TSS

