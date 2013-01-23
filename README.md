reflow-oven-controller
======================

Yet another reflow oven controller for [c-base](http://c-base.org). The design is mainly based on parts we had available in c-lab. *Note: the project is work in progress, major fuckups may be present in the design files*

The design is modular:
- MCU board has a microcontroller, a solid state relay and a header for analog temperature sensor daughterboards. Headers for ISP programming, SPI communication and CP2102 USB-to-USART module are available too. SSR used is S102S02.
- a daughterboard for PT100 sensor has all the analog sensor conditioning circuitry: constant current source, wire resistance elimination, active filter. Based on Microchip App Note [AN687](http://www.microchip.com/stellent/idcplg?IdcService=SS_GET_PAGE&nodeId=1824&appnote=en011700)

UI features available
---------------------

- power LED
- "heater on" LED
- "oven running" LED
- "start/stop" button
- SPI header for serial LCD module 


