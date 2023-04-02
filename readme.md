# CANalyze Firmware for STM32F072
This repository contains the CANalyze firmware, which has been ported from the original implementation available 
at [CANalyze](https://github.com/kkuchera/canalyze-fw) to run on the STM32F072 microcontroller. You can use the 
Blue Pill board for testing by replacing the STM32F103 chip with an STM32F072. The reason for this replacement 
is that the STM32F103 does not allow simultaneous access to both USB and CAN peripherals.

## Getting started
Bring up CAN interface
```shell
$ sudo ip link set can0 up type can bitrate 500000
```
Sniff CAN messages
```shell
$ cansniffer -c can0
```
or dump all CAN messages
```shell
$ candump can0
```
Send a CAN message
```shell
$ cansend can0 666#01020304
```
