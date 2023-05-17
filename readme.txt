Bootloader to stm32f4 will be built by STM32IDE
Guide for Host App PC from ./HostApp/PcTool/readme.txt (setup MinGW to build on Window system)
./HostApp/PcTool/bsp.bin => blink 4 led on stm32f4 dis kit, which is mapped vector table offset to new application partition in Linker script (at 0x08040000), use this one to test