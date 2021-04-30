# No-MCU Ultrasonic Distance Meter

A Distance Meter that uses no MCU. Bases on ultrasonic ranging module [HC-SR04](https://cdn.sparkfun.com/datasheets/Sensors/Proximity/HCSR04.pdf).

The schematic employs just three standard logic ICs (8-bit counter, divider/oscillator, quad NAND) and a ROM chip (acts as a binary-to-decimal converter).

<a href="https://www.tindie.com/stores/alexblack/?ref=offsite_badges&utm_source=sellers_AlexBlack&utm_medium=badges&utm_campaign=badge_medium"><img src="https://d2ss6ovg47m0r5.cloudfront.net/badges/tindie-mediums.png" alt="I sell on Tindie" width="150" height="78"></a>

## Schematic

![](no-mcu-distance-meter-sch.png)

## Photos

![](photo/dm-breadboard.jpg)

![](photo/dm-pcb-1.jpg)

![](photo/dm-pcb-2.jpg)

![](photo/dm-pcb-3.jpg)

## Video

[![IMAGE ALT TEXT](http://img.youtube.com/vi/FPoeiAQJhnc/0.jpg)](https://www.youtube.com/watch?v=FPoeiAQJhnc)

## Project files

- `no-mcu-distance-meter-kicad5-prj/`  - KiCAD 5.1 project with the schematic and PCB design
- `eprom-programmer`  - Arduino sketch to program the ROM chip using mega2560 (see the sketch for the connections info).

## License

[CC BY-NC 4.0](https://creativecommons.org/licenses/by-nc/4.0/) © github.com/AlexIII
